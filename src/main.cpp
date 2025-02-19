#include <dpp/dpp.h>
#include "admin.hpp"
#include "commands.hpp"
#include "macros.hpp"

constexpr bool clearCommands = false;

int main() {
    admins::Initialize();
    macros::Initialize();

    std::string token;
    std::ifstream tokenFile("token.txt");
    if (tokenFile.is_open()) {
        std::getline(tokenFile, token);
        tokenFile.close();
    } else {
        std::cerr << "Failed to open token file" << std::endl;
        return 1;
    }

    dpp::cluster bot(token, dpp::i_default_intents | dpp::i_message_content);
    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "settings") {
            commands::Settings(bot, event);
        } else if (event.command.get_command_name() == "add_admin") {
            admins::AdminCommand(commands::AddAdmin, bot, event);
        } else if (event.command.get_command_name() == "remove_admin") {
            admins::AdminCommand(commands::RemoveAdmin, bot, event);
        } else if (event.command.get_command_name() == "add_macro") {
            admins::AdminCommand(commands::AddMacro, bot, event);
        } else if (event.command.get_command_name() == "remove_macro") {
            admins::AdminCommand(commands::RemoveMacro, bot, event);
        } else if (event.command.get_command_name() == "download") {
            commands::Download(bot, event);
        }
    });

    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (clearCommands && dpp::run_once<struct clear_bot_commands>()) {
            bot.global_bulk_command_delete();
        }

        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand logFileCommand("settings", "Upload your Panda3DS settings file in a nicely formatted manner", bot.me.id);
            logFileCommand.add_option(
                dpp::command_option(dpp::co_attachment, "file", "The Panda3DS settings file", true)
            );
    
            dpp::slashcommand addAdminCommand("add_admin", "Add a user to the list of admins", bot.me.id);
            addAdminCommand.add_option(
                dpp::command_option(dpp::co_user, "user", "The user to add as an admin", true)
            );

            dpp::slashcommand removeAdminCommand("remove_admin", "Remove a user from the list of admins", bot.me.id);
            removeAdminCommand.add_option(
                dpp::command_option(dpp::co_user, "user", "The user to remove as an admin", true)
            );

            dpp::slashcommand addMacroCommand("add_macro", "Add a macro to the list of macros", bot.me.id);
            addMacroCommand.add_option(
                dpp::command_option(dpp::co_string, "name", "The name of the macro", true)
            ).add_option(
                dpp::command_option(dpp::co_string, "response", "The response of the macro", true)
            );

            dpp::slashcommand removeMacroCommand("remove_macro", "Remove a macro from the list of macros", bot.me.id);
            removeMacroCommand.add_option(
                dpp::command_option(dpp::co_string, "name", "The name of the macro", true)
            );

            dpp::slashcommand downloadCommand("download", "Show download links for Panda3DS", bot.me.id);

            bot.global_command_create(logFileCommand);
            bot.global_command_create(addAdminCommand);
            bot.global_command_create(removeAdminCommand);
            bot.global_command_create(addMacroCommand);
            bot.global_command_create(removeMacroCommand);
            bot.global_command_create(downloadCommand);
        }
    });

    bot.on_message_create([&bot](const dpp::message_create_t& event) {
        macros::CheckForMacro(event);
    });
    
    bot.start(dpp::st_wait);
}
