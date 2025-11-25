#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "protocol.h"
#include "string_func.h"
#include <bitset>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

std::vector<std::string> MainWindow::parse_cmd ( std::string command ) {
    std::vector<std::string> res = {"SUCCESS"};
    std::bitset<2> flags = {0};
    enum console_parse_flags {
        QUOTE = 0,
        VAR   = 1,
        SPACE = 2,
        ESC   = 3,
    };

    int s = 0;
    int v;
    int i = 0;
    for (; i < command.size() ; i++ ) {
        if ( command[i] == ' ' ) {
            if ( flags[SPACE] ) {
                s++;
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
        } else {
            flags[SPACE] = false;
        }
    }
    if ( flags[QUOTE] ) res[0] = "PARSE ERROR: Unterminated quote";
    else if ( flags[VAR] ) {
        if ( i != s+1 );
        std::string var = command.substr(s+1, i-s-1);
        try {
            res.push_back(conVars[var]);
        } catch ( std::out_of_range ) {
            res[0] = "PARSE ERROR: No variable named \"" + var + "\"";
        }
    } else res.push_back(command.substr(s, i-s));

    return res;
}

std::pair<bool, std::string> MainWindow::conInterpret ( std::string command ) {
    int size;
    std::string ans;
    bool success;

    std::vector<std::string> cmd = parse_cmd(command);
    if ( cmd.size() == 1 ) return std::pair(false, "");

    if ( cmd[0] != "SUCCESS" ) {
        ans = cmd[0];
        success = false;
    } else if ( cmd.size() >= 4 && cmd[2] == "=" ) {
        size = 4;

        conVars.insert({cmd[1], cmd[3]});
        ans = "";
    } else if ( cmd.size() >= 3 && cmd[1] == "print" ) {
        size = 3;

        try {
            ans = conVars[cmd[2]];
            success = true;
        } catch ( std::out_of_range ) {
            ans = "";
            success = false;
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "printall" ) {
        size = 2;

        ans = "";
        for ( auto var : conVars ) {
            if ( var.second != "" )
                ans += var.first + ": " + var.second + '\n';
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "clear" ) {
        size = 2;

        ans = "";
        success = true;
        ui->ConsoleOut->setText("");
    } else if ( cmd.size() >= 3 && cmd[1] == "connect" ) {
        size = 3;

        bool succ;
        // protocol::conn = Connection(cmd[1].data(), 8494, &succ);

        if ( succ ) {
            ans = "Connected";
            success = true;
        }
        else {
            ans = "Failed to connect";
            success = false;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "join" ) {
        size = 3;

        char pass[9] = {0};
        protocol::ErrorCode res = protocol::join(cmd[1].data(), pass);
        switch ( res ) {
            case protocol::NOERROR:
                ans = pass;
                success = true;
                break;
            case protocol::RENAME:
                ans = "Rename";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "Already joined";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "Failed to send";
                success = false;
                break;
            default:
                ans = "Unexpected error";
                success = false;
                break;
        }
    } else {
        ans = "ERROR: Unknown command \""+cmd[1]+"\"";
        success = false;
    }
    return std::pair(success, ans);
}

void MainWindow::conOut ( std::string text ) {
    if ( text != "" )
    ui->ConsoleOut->setText(ui->ConsoleOut->text() + QString::fromStdString(text) + '\n');
}

void MainWindow::on_ConsoleIn_returnPressed()
{
    QString command = ui->ConsoleIn->text();
    ui->ConsoleIn->setText("");
    conOut("> "+command.toStdString());
    std::pair<int, std::string> result = conInterpret(command.toStdString());
    conOut(result.second);
}
