#pragma once

#include <iostream>
#include <memory>
#include "Simulation.h"
#include "_libs/_logger.h"

class SimulationController
{
public:
    const std::string COMMAND_STEP = "step";
    const std::string COMMAND_EXIT = "exit";

    SimulationController(std::shared_ptr<Simulation> sim, bool isTestMod, bool isParallel)
            : simulation(std::move(sim)),
            isTestMod(isTestMod),
            isParallel(isParallel)
    {};

    void RunSimulation()
    {
        CheckSimulation();

        if (isParallel)
        {
            RunParallelSimulation();
        }
        else if (!isTestMod)
        {
            RunSequentialSimulation();
        }
        else
        {
            RunTestModeSimulation();
        }
    }

    void StopSimulation()
    {
        CheckSimulation();

        simulation->Stop();
    }

private:
    std::shared_ptr<Simulation> simulation = nullptr;
    bool isTestMod = false;
    bool isParallel = false;

    void CheckSimulation()
    {
        if (!simulation) throw std::runtime_error("no simulation");
    }

    void RunParallelSimulation()
    {
        Logger::Println("Starting simulation in parallel mode...");
        simulation->ExecuteParallel();
    }

    void RunSequentialSimulation()
    {
        Logger::Println("Starting simulation in sequential mode...");
        simulation->ExecuteSequential();
    }

    void RunTestModeSimulation()
    {
        Logger::Println("Starting simulation in sequential test mode...");
        Logger::Println("Usage:");
        Logger::Println("\t<" + COMMAND_STEP + ">: runs simulation step");
        Logger::Println("\t<" + COMMAND_EXIT + ">: exits from simulation");

        bool shouldExit = false;
        while (!shouldExit)
        {
            std::string command;
            std::getline(std::cin, command);
            if (command == COMMAND_STEP)
            {
                simulation->ExecuteSequentialStep();
            }
            else if (command == COMMAND_EXIT)
            {
                shouldExit = true;
            }
        }
    }
};
