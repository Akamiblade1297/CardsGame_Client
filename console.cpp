#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "protocol.h"
#include "string_func.h"
#include "functions.h"
#include <bitset>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

std::vector<std::string> MainWindow::parse_cmd ( std::string& command ) {
    std::vector<std::string> res = {"SUCCESS"};
    std::bitset<6> flags = {0};
    enum console_parse_flags {
        QUOTE = 0,
        VAR   = 1,
        SPACE = 2,
        ESC   = 3,
        QUOTED= 4,
        PARTED= 5,
    };

    int s = 0;
    int v;
    int i = 0;
    for (; i < command.size() ; i++ ) {
        if ( command[i] == ' ' ) {
            if ( flags[SPACE] || i == 0 ) {
                command.erase(i, 1);
                i--;
                continue;
            }
            flags[SPACE] = true;
            if ( flags[QUOTE] ) {
                if ( flags[VAR] ) {
                    flags[VAR] = false;
                    std::string var = conVars[command.substr(v, i-v)];
                    int rsize = i-v+1;
                    command.replace(v-1, rsize, var);
                    i += var.size() - rsize + 1;
                }
            } else if ( !flags[VAR] ) {
                res.push_back(command.substr(s, i-s));
                s = i+1;
            } else {
                flags[QUOTED] = false;
                flags[VAR] = false;
                std::string var = command.substr(v, i-v);
                res.push_back(conVars[var]);
                s = i+1;
            }
        } else if ( command[i] == '"' ) {
            if ( flags[QUOTE] ) {
                flags[QUOTE] = false;
                if ( i != s+1 ) {
                    std::string quoted = command.substr(s+1, i-s-1);
                    if ( flags[VAR] ) {
                        int b = v-s-1;
                        std::string var = quoted.substr(b, quoted.size()-b);
                        quoted = quoted.substr(0, b-1) + conVars[var] ;
                    }
                    res.push_back(quoted);
                    flags[QUOTED] = true;
                }

            } else if ( flags[SPACE] ) {
                flags[QUOTE] = true;
            } else {
                res[0] = "PARSE ERROR: Unexpected '\"' token";
                break;
            }
            flags[SPACE] = false;
        } else if ( command[i] == '$' ) {
            if ( flags[SPACE] || command[i-1] == '"' ) {
                flags[VAR] = true;
                flags[SPACE] = false;
                v = i+1;
            } else {
                res[0] = "PARSE ERROR: Unexpected '$' token";
                return res;
            }
        } else if ( command[i] == ';' ) {
            flags[PARTED] = true;
            if ( command.size() > i+1 && command[i+1] == ' ' && flags[SPACE] ) {
                command = command.substr(i, command.size()-i+1);
            } else {
                res[0] = "PARSE ERROR: Unexpected ';' token";
                return res;
            }
            break;
        } else if ( command[i] == '&' || command[i] == '|' )  {
            flags[PARTED] = true;
            if ( command.size() > i+2 && command[i+1] == command[i] && command[i+2] == ' ' && flags[SPACE] ) {
                command = command.substr(i+1, command.size()-i);
            } else {
                res[0] = "PARSE ERROR: Unexpected '"+std::string(1,command[i])+"' token";
            }
            break;
        } else {
            flags[SPACE] = false;
        }
    }
    if ( flags[QUOTE] ) res[0] = "PARSE ERROR: Unterminated quote";
    else if ( flags[VAR] ) {
        if ( i != s+1 ) {
            std::string var = command.substr(s+1, i-s-1);
            res.push_back(conVars[var]);
        }
    } else if ( !( flags[QUOTED] || flags[PARTED] ) ) res.push_back(command.substr(s, i-s));

    if ( ! flags[PARTED] ) command = "";
    return res;
}

std::pair<bool, std::string> MainWindow::conInterpret ( std::string command ) {
    int cmdsize = -1;
    std::string ans;
    bool success;

    std::vector<std::string> cmd = parse_cmd(command);
    if ( cmd.size() == 1 || cmd[1] == "" ) return std::pair(true, "");
    std::cout << command << std::endl;
    for ( const auto& s : cmd ) std::cout << s << ' '; std::cout << std::endl;

    if ( cmd[0] != "SUCCESS" ) {
        std::string ans = cmd[0];
        bool success = false;
        return std::pair(success, ans);
    } else if ( cmd.size() >= 4 && cmd[2] == "=" ) { // [var] = [val]
        cmdsize = 4;

        if ( conVars.find(cmd[1]) == conVars.end() )
            conVars.insert({cmd[1], cmd[3]});
        else
            conVars[cmd[1]] = cmd[3];
        ans = "";
        success = true;
    } else if ( cmd.size() >= 3 && cmd[1] == "print" ) { // print [var]
        cmdsize = 3;

        ans = conVars[cmd[2]];
        success = true;
    } else if ( cmd.size() >= 2 && cmd[1] == "printall" ) { // printall
        cmdsize = 2;

        std::vector<std::string> vars;
        for ( const auto& var : conVars ) {
            if ( var.second != "" )
                vars.push_back( var.first + ": " + var.second );
        }
        ans = join(vars, "\n");
        ans = ans.substr(1, ans.size()-1);
        success = true;
    } else if ( cmd.size() >= 2 && cmd[1] == "clear" ) { // clear
        cmdsize = 2;

        ans = "CLEAR";
        success = true;
    } else if ( cmd.size() >= 4 && cmd[1] == "connect" ) { // connect [addr] [port]
        cmdsize = 4;

        try {
            protocol::ErrorCode res = protocol::connect(cmd[2].data(), (unsigned short)std::strtoul(cmd[3].c_str(), NULL, 0));

            if ( res == protocol::NOERROR ) {
                ans = "Connected";
                success = true;
            } else {
                ans = "ERROR: Failed to connect";
                success = false;
            }
        } catch ( std::out_of_range ) {
            ans = "Invalid port";
            success = false;
        } catch ( std::invalid_argument ) {
            ans = "Invalid port";
            success = false;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "join" ) { // join [name]
        cmdsize = 3;

        char pass[8] = {0};
        protocol::ErrorCode res = protocol::join(cmd[2].data(), pass);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                ans.append(pass, 8);
                success = true;
                break;
            case protocol::RENAME:
                ans = "RENAME";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Already joined";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "rejoin" ) { // rejoin [pass]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::rejoin(cmd[2].data());
        switch ( res ) {
            case protocol::NOERROR:
                ans = "Rejoined successfully";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: No player with given pass";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Already joined";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "ping" ) { // ping
        cmdsize = 2;

        int time;
        protocol::ErrorCode res = protocol::ping(time);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "Received data. Elapsed time: "+std::to_string(time)+"ms";
                success = true;
                break;
            case protocol::TIMEOUT:
                ans = "ERROR: Didn't receive data. Timeout exceeded: "+std::to_string(time)+"ms";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send data";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "chat" ) { // chat [msg]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::chat(cmd[2]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "act" ) { // act [action]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::action(cmd[2]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "whisper" ) { // whisper [player] [msg]
        cmdsize = 4;

        protocol::ErrorCode res = protocol::whisper(cmd[2], cmd[3]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: No such player named \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "roll" ) { // roll [dice] [num]
        cmdsize = 4;

        protocol::ErrorCode res = protocol::roll(cmd[2], cmd[3]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 6 && cmd[1] == "deck" ) { // deck [src] [dest] [x] [y]
        cmdsize = 6;
        Deck* deck = deckByName(cmd[2]);
        if ( deck == nullptr ) {
            ans = "ERROR: Not found deck";
            success = false;
        } else if ( deck->Cards.empty() ) {
            ans = "ERROR: Local deck is empty. Try running \"cards "+cmd[2]+"\"";
            success = false;
        } else {
            protocol::ErrorCode res = protocol::deck(cmd[2], cmd[3], cmd[4], cmd[5]);
            switch ( res ) {
                case protocol::NOERROR:
                    ans = "";
                    success = true;
                    break;
                case protocol::EMPTY:
                    ans = "ERROR: The deck is empty";
                    success = false;
                    break;
                case protocol::NOT_FOUND:
                    ans = "ERROR: Not found destination or server version mismatched with client version";
                    success = false;
                    break;
                case protocol::NOT_A_NUM:
                    ans = "ERROR: Invalid number";
                    success = false;
                    break;
                case protocol::OUT_OF_RNG:
                    ans = "ERROR: Out of Range";
                    success = false;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to send";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
                    break;
            }
        }
    } else if ( cmd.size() >= 7 && cmd[1] == "move" ) { // move [src] [index] [dest] [x] [y]
        cmdsize = 7;
        CardContainer* spatial;
        if ( spatial == nullptr ) {
            ans = "ERROR: Not found source";
            success = false;

        }
        protocol::ErrorCode res = protocol::move(cmd[2], cmd[3], cmd[4], cmd[5], cmd[6]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: Not found source or destination";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 5 && cmd[1] == "rotate" ) { // [container] [index] [rot]
        cmdsize = 5;

        protocol::ErrorCode res = protocol::rotate(cmd[2], cmd[3], cmd[4]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: Not such spatial container \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "flip" ) { // flip [container] [index]
        cmdsize = 4;

        protocol::ErrorCode res = protocol::flip(cmd[2], cmd[3]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: Not such spatial container \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "shuffle" ) { // shuffle [deck]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::shuffle(cmd[2]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: No such deck \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "set" ) { // set [stat] [value]
        cmdsize = 4;

        protocol::ErrorCode res = protocol::set(cmd[2], cmd[3]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: No such stat \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "rename" ) { // rename [name]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::rename(cmd[2]);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "Renamed to "+cmd[2];
                success = true;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "see" ) { // see [visible]
        cmdsize = 3;

        CardContainer* spatial = spatialByName(cmd[2], protocol::localplayer);
        Player* plr = playerMgr.playerByName(cmd[2]);
        std::vector<Card>* cards = nullptr;

        if ( spatial == nullptr ) {
            if ( plr == nullptr ) {
                ans = "ERROR: No such spatial or player \""+cmd[2]+"\"";
                success = false;
            } else {
                cards = &plr->Equiped.Cards;
            }
        } else cards = &spatial->Cards;

        if ( cards != nullptr ) {
            int old_size = cards->size();
            protocol::ErrorCode res = protocol::see(cmd[2], cards);

            switch ( res ) {
                case protocol::NOERROR:
                    cards->erase(cards->begin(), cards->begin()+old_size);
                    ans = "ID:\tNumber:\tX:\tY:\tRotation:\n\n";
                    for ( int i = 0 ; i < cards->size() ; i++ ) {
                        Card crd = cards->at(i);
                        ans += std::to_string(i) + '\t' + crd.unparse_card() + '\t' + std::to_string(crd.X) + '\t' + std::to_string(crd.Y) + '\t' + std::to_string(crd.Rotation) + '\n';
                    }
                    success = true;
                    break;
                case protocol::NOT_FOUND:
                    ans = "ERROR: Not found container. Client version probably mismatching Server version";
                    success = false;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to send";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
                    break;
            }
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "cards" ) { // cards [nonvisible]
        cmdsize = 3;

        Deck* deck = deckByName(cmd[2]);
        Player* plr = playerMgr.playerByName(cmd[2]);
        std::vector<Card>* cards = nullptr;

        if ( deck == nullptr ) {
            if ( plr == nullptr ) {
                ans = "ERROR: No such deck or player \""+cmd[2]+"\"";
                success = false;
            } else {
                cards = &plr->Inventory.Cards;
            }
        } else cards = &deck->Cards;

        if ( cards != nullptr ) {
            int old_size = cards->size();
            protocol::ErrorCode res = protocol::cards(cmd[2], cards);

            switch ( res ) {
                case protocol::NOERROR:
                    cards->erase(cards->begin(), cards->begin()+old_size);
                    ans = "ID:\tNumber:\tX:\tY:\tRotation:\n\n";
                    for ( int i = 0 ; i < cards->size() ; i++ ) {
                        Card crd = cards->at(i);
                        ans += std::to_string(i) + '\t' + crd.unparse_card() + '\t' + std::to_string(crd.X) + '\t' + std::to_string(crd.Y) + '\t' + std::to_string(crd.Rotation) + '\n';
                    }
                    success = true;
                    break;
                case protocol::NOT_FOUND:
                    ans = "ERROR: Not found container. Client version probably mismatching Server version";
                    success = false;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to send";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
                    break;
            }
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "stat" ) { // stat [player]
        cmdsize = 3;

        Player* plr = playerMgr.playerByName(cmd[2]);

        if ( plr == nullptr ) {
            ans = "ERROR: No such player \""+cmd[2]+"\"";
            success = false;
        } else {
            std::string stats[3];
            protocol::ErrorCode res = protocol::stat(cmd[2], stats);

            switch ( res ) {
                case protocol::NOERROR:
                    plr->Level=stats[0]; plr->Power=stats[1]; plr->Gold=stats[2];
                    ans = "LEVEL: "+stats[0]+"\nPOWER: "+stats[1]+"\nGOLD: "+stats[2];
                    success = true;
                    break;
                case protocol::NOT_FOUND:
                    ans = "ERROR: Not found container. Client version probably mismatching Server version";
                    success = false;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to send";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
                    break;
            }
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "players" ) { // players
        cmdsize = 2;

        std::vector<std::string> plrs;
        protocol::ErrorCode res = protocol::players(&plrs);
        switch ( res ) {
            case protocol::NOERROR:
                ans = "";
                for ( int i = 0 ; i < plrs.size() ; i++ )
                    ans += plrs[i] + '\n';
                success = true;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else {
        ans = "ERROR: Unknown command \""+cmd[1]+"\"";
        success = false;
    }

    if ( cmd.size() > cmdsize && cmdsize != -1 ) {
        if ( cmd[cmdsize] == ">" && cmd.size() > cmdsize+1 ) {
            if ( conVars.find(cmd[cmdsize+1]) == conVars.end() )
                conVars.insert({cmd[cmdsize+1], ans});
            else
                conVars[cmd[cmdsize+1]] = ans;
            ans = "";

            if ( cmd.size() > cmdsize+2 ) ans += "\nWarning: Unexpected token: \""+cmd[cmdsize]+"\"";
        } else ans += "\nWarning: Unexpected token: \""+cmd[cmdsize]+"\"";
    }
    if ( command != "" ) {
        if ( command[0] == ';' || ( command[0] == '&' && success ) || ( command[0] == '|' && !success ) ) {
            auto [success2, ans2] = conInterpret(command.substr(1, command.size()-1));
            success = success2;
            if ( ans2 == "CLEAR" ) ans = ans2;
            else ans += ( ( ans == "" || ans2 == "" ) ? "" : "\n" ) + ans2;
        }
    }
    return std::pair(success, ans);
}

void MainWindow::conOut ( QString text ) {
    if ( text != "" )
    ui->ConsoleOut->setText(ui->ConsoleOut->text() + text + '\n');
}

void MainWindow::conIn ( QString command ) {
    conOut("> "+command);
    std::pair<int, std::string> result = conInterpret(command.toStdString());
    if ( result.second == "CLEAR" ) ui->ConsoleOut->setText("");
    else conOut(QString::fromStdString((result.second)));
}

void MainWindow::on_ConsoleIn_returnPressed()
{
    QString command = ui->ConsoleIn->text();
    ui->ConsoleIn->setText("");

    conIn(command);
}
