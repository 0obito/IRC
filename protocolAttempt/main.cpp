// so I'll have something like this:

#include <iostream>
#include <string>

/*
    exmaple for a structure that will contain the 'parsed' client message.
    'parsed' means it was checked for syntax errors, and tokenized into the structure content.
    I expect something similar to come from nisrine :p
*/
struct parsedMessage {
    std::string prefix;     // if the client sends it ofc
    std::string command;
    std::vector<std::string> parameters;
};


/*
    I am thinking of something like this for the handle functions.
    the prototype should be 'uniform'
    'uniform' means each handle_function() will take the same args, and return the same type.
*/
typedef void (*commandHandler)(Server&, Client&, parsedMessage&);


/*
    handle_function examples.
*/
void handleJoin(Server& server, Client& client, parsedMessage& msg) {
    // something;
}
void handlePrivmsg(Server& server, Client& client, parsedMessage& msg) {
    // something else;
}
// etc..


/*
    this is the command dispatcher class.
    it's where the magic happens.
*/
class commandDispatcher {
    private:
        /*
            this simply maps a string to a function pointer, like a "key = value" thing.
        */
        std::map<std::string, commandHandler> _handlers;

    public:
        /*
            this is the default constructor ofc.
        */
        commandDispatcher() {
            _handlers["JOIN"] = &handleJoin;
            _handlers["PRIVMSG"] = &handlePrivmsg;
            // etc..
        }

        /*
            This will be our method that routes the execution.
            It's dumb (lol) and it has 0 logic.
            It simply checks the map, and call the "smart" people to do the work.
        */
        void routeCommand(Server& server, Client& client, parsedMessage& msg) {
            
            if (_handlers.find(msg.command) != _handlers.end()) {
                commandHandler func = _handlers[msg.command];
                func(server, client, msg);
            }
            else {
                std::cout << "Send to Client: 421 ERR_UNKNOWNCOMMAND" << std::endl;
            }
        }
};
