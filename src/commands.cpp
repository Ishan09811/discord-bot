#include "commands.hpp"
#include "admin.hpp"
#include "macros.hpp"
#include <dpp/dpp.h>
#include <toml.hpp>

std::string boolToCheckbox(bool value) {
    return value ? "☑" : "☐";
}

unsigned constexpr str_hash(char const *input) {
    if (!*input)
        return 0;

    return *input ?
        static_cast<unsigned int>(*input) + 33 * str_hash(input + 1) :
        5381;
}

namespace commands {

    void Settings(dpp::cluster& bot, const dpp::slashcommand_t& event) {
        dpp::snowflake fileId = std::get<dpp::snowflake>(event.get_parameter("file"));
        dpp::attachment att = event.command.get_resolved_attachment(fileId);
        bot.request(att.url, dpp::http_method::m_get, [event](const dpp::http_request_completion_t& completion) {
            if (completion.status != 200) {
                event.reply(dpp::message("Failed to download file").set_flags(dpp::m_ephemeral));
                return;
            }

            if (completion.body.size() == 0) {
                event.reply(dpp::message("File size is 0").set_flags(dpp::m_ephemeral));
                return;
            }
            
            std::string rendererPretty = "unknown";
            std::string dsp = "unknown";
            bool isShaderJit = false;

            try {
                std::string toml(completion.body.begin(), completion.body.end());
                std::istringstream is(toml, std::ios_base::binary | std::ios_base::in);
                auto data = toml::parse(is);

                if (data.count("GPU") != 0) {
                    const auto& gpu = toml::find(data, "GPU");
                    const std::string& renderer = toml::find(gpu, "Renderer").as_string();
                    isShaderJit = toml::find(gpu, "EnableShaderJIT").as_boolean();
                    switch (str_hash(renderer.c_str())) {
                        case str_hash("vulkan"):
                            rendererPretty = "<:vulkant:1148648218658340894>";
                            break;
                        case str_hash("opengl"):
                            rendererPretty = "<:opengl:1148648565879603231>";
                            break;
                        default:
                            rendererPretty = "null";
                            break;
                    }
                }

                if (data.count("Audio") != 0) {
                    const auto& audio = toml::find(data, "Audio");
                    dsp = toml::find(audio, "DSPEmulation").as_string();
                }
            } catch (const std::exception& e) {
                event.reply(dpp::message("Failed to parse file").set_flags(dpp::m_ephemeral));
                return;
            }

            dpp::embed embed = dpp::embed()
                .set_color(dpp::colors::sti_blue)
                .set_author(event.command.get_issuing_user().global_name + "'s settings", "", event.command.get_issuing_user().get_avatar_url())
                .set_thumbnail("https://panda3ds.com/images/panda-icon.png")
                .add_field("", "Renderer: " + rendererPretty)
                .add_field("", "Shader JIT: " + boolToCheckbox(isShaderJit))
                .add_field("", "DSP Emulation: " + dsp);

            dpp::message msg(event.command.channel_id, embed);
            event.reply(msg);
        });
    }

    void AddAdmin(dpp::cluster &bot, const dpp::slashcommand_t &event) {
        dpp::guild_member member = event.command.get_resolved_member(std::get<dpp::snowflake>(event.get_parameter("user")));
        admins::AddAdmin(event, member);
    }

    void RemoveAdmin(dpp::cluster &bot, const dpp::slashcommand_t &event) {
        dpp::guild_member member = event.command.get_resolved_member(std::get<dpp::snowflake>(event.get_parameter("user")));
        admins::RemoveAdmin(event, member);
    }

    void AddMacro(dpp::cluster &bot, const dpp::slashcommand_t &event) {
        std::string name = std::get<std::string>(event.get_parameter("name"));
        std::string response = std::get<std::string>(event.get_parameter("response"));

        if (name.find('\n') != std::string::npos || response.find('\n') != std::string::npos) {
            event.reply(dpp::message("Macro cannot contain '\n' as it's used as a separator internally").set_flags(dpp::m_ephemeral));
            return;
        }

        bool added = macros::AddMacro(name, response);
        event.reply(dpp::message(added ? "Macro added" : "Macro already exists").set_flags(dpp::m_ephemeral));
    }

    void RemoveMacro(dpp::cluster &bot, const dpp::slashcommand_t &event) {
        std::string name = std::get<std::string>(event.get_parameter("name"));
        bool removed = macros::RemoveMacro(name);
        event.reply(dpp::message(removed ? "Macro removed" : "Macro does not exist").set_flags(dpp::m_ephemeral));
    }

    void Download(dpp::cluster &bot, const dpp::slashcommand_t &event) {
        dpp::embed embed = dpp::embed()
            .set_color(dpp::colors::sti_blue)
            .set_author("Download", "", "https://panda3ds.com/images/panda-icon.png")
            .set_thumbnail("https://panda3ds.com/images/panda-icon.png")
            .set_description("Download the latest version of Panda3DS [here](https://panda3ds.com/download.html)");
        event.reply(dpp::message(event.command.channel_id, embed));
    }

}