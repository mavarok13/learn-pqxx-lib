#pragma once

#include <map>
#include <functional>

namespace console_handler {

using Handler = std::function<void(std::istream&, std::ostream&)>;

class ConsoleHandler {
public:
    void AddAction(const std::string & command, const std::string & args, const std::string & description, Handler && handler) {
        actions_.try_emplace(std::move(command), std::move(ActionInfo{args, description, handler}));
    }

    [[no_discard]] bool ParseCommand(std::istream & is, std::ostream & os) {
        if (std::string command_name; is >> command_name) {
            if (auto it = actions_.find(command_name); it != actions_.end()) {
                it->second.handler(is, os);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    void PrintCommandsInfo(std::ostream & os) {
        os << "Commands:" << std::endl;
        for (auto command_it = actions_.begin(); command_it != actions_.end(); ++command_it) {
            os << "|- " << command_it->first << " " << command_it->second.args << " " << command_it->second.description << std::endl;
        }
    }
private:
    struct ActionInfo {
    public:
        std::string args;
        std::string description;
        Handler handler;
    };

    std::map<std::string, ActionInfo> actions_;
};

} //namespace console_handler