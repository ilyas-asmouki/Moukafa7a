#include <iostream>
#include <vector>
#include "terminal_command_interface.h"

TerminalCommandInterface::TerminalCommandInterface() : Node("terminal_command_interface")
{
    command_publisher = this->create_publisher<motor_controller::msg::MotorCommand>("/motor_commands", 10);
    std::thread([this]() { this->listen(); }).detach();
}

void TerminalCommandInterface::listen()
{
    std::string line;
    std::cout << "Connection to node successful. Enter commands (e.g., 'fl 149 rr 19 fr 218' or 'fw 91'): " << std::endl;
    
    while (std::getline(std::cin, line))
    {
        std::istringstream iss(line);
        std::string motor;
        int velocity;
        bool invalid = false;

        int fr_velocity = 0, fl_velocity = 0, rr_velocity = 0, rl_velocity = 0;

        while (iss >> motor) 
        {
            if (!(iss >> velocity) and motor != "st") {
                std::cout << "Missing velocity for " << motor << std::endl;
                invalid = true;
                break;
            }

            if (motor == "fr") {
                fr_velocity = velocity;
            } else if (motor == "fl") {
                fl_velocity = velocity;
            } else if (motor == "rr") {
                rr_velocity = velocity;
            } else if (motor == "rl") {
                rl_velocity = velocity;
            } else if (motor == "fw") {  // forward: set both front motors to the same velocity
                fr_velocity = velocity;
                fl_velocity = velocity;
                rr_velocity = velocity;
                rl_velocity = velocity;
            } else if (motor == "bw") {  // backward: set both front motors to the same negative velocity
                fr_velocity = -velocity;
                fl_velocity = -velocity;
                rr_velocity = -velocity;
                rl_velocity = -velocity;
            } else if (motor == "rt") {  // rotate: set front and rear motors to opposite velocities for rotation
                fr_velocity = velocity;
                fl_velocity = -velocity;
                rr_velocity = velocity;
                rl_velocity = -velocity;
            } else if (motor == "tn") {  // turn: set rear motors, use negative velocity for left turn
                rr_velocity = velocity;
                rl_velocity = -velocity;
            } else if (motor == "st") {  // stop: set all motors to 0
                fr_velocity = 0;
                fl_velocity = 0;
                rr_velocity = 0;
                rl_velocity = 0;
            } else {
                std::cout << "Invalid direction: " << motor << std::endl;
                invalid = true;
                break;
            }
        }

        if (!invalid) {
            std::ostringstream oss;
            oss << "fr " << fr_velocity << " fl " << fl_velocity << " rr " << rr_velocity << " rl " << rl_velocity;
            send_motor_command(oss.str());
        }

        std::cout << "Enter commands: " << std::endl;
    }
}

void TerminalCommandInterface::send_motor_command(const std::string &command)
{
    auto message = motor_controller::msg::MotorCommand();
    message.motor = command;  // use entire command string (to be changed later)
    command_publisher->publish(message);
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TerminalCommandInterface>());
    rclcpp::shutdown();
    return 0;
}
