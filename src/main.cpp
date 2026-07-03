#include "cosmos_wam/pipeline_scheduler.h"
#include <iostream>
#include <memory>

void printUsage() {
  std::cout << "Usage:\n"
            << "  Local Mode:  ./cosmos_wam_client local\n"
            << "  Remote Mode: ./cosmos_wam_client <host> <user> <password> "
               "[local_image_path] [duration] [fps]\n\n"
            << "Defaults (for optional remote args):\n"
            << "  local_image_path:  ./input/InputImage.png\n"
            << "  duration:          3.0\n"
            << "  fps:               5\n"
            << std::endl;
}

int main(int argc, char *argv[]) {
  std::cout << "========================================================="
            << std::endl;
  std::cout << "         CosmosWAM Modular remote C++ Client            "
            << std::endl;
  std::cout << "========================================================="
            << std::endl;

  std::string host = "192.168.50.254";
  std::string user = "";
  std::string pass = "";
  std::string image_path = "./input/InputImage.png";
  float duration = 3.0f;
  int fps = 5;
  float initial_velocity = 15.0f;

  if (argc > 1) {
    if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
      printUsage();
      return 0;
    }
    host = argv[1];
  }

  if (host != "local" && argc < 4) {
    std::cerr << "Error: <host>, <user>, and <password> are required for remote end-to-end execution." << std::endl;
    printUsage();
    return 1;
  }

  if (argc > 2)
    user = argv[2];
  if (argc > 3)
    pass = argv[3];
  if (argc > 4)
    image_path = argv[4];
  if (argc > 5)
    duration = std::stof(argv[5]);
  if (argc > 6)
    fps = std::stoi(argv[6]);
  if (argc > 7)
    initial_velocity = std::stof(argv[7]);

  std::cout << "Target Host: " << host << std::endl;
  if (host != "local") {
    std::cout << "Target User: " << user << std::endl;
  }
  std::cout << "Input Image: " << image_path << std::endl;
  std::cout << "Duration:    " << duration << " seconds" << std::endl;
  std::cout << "FPS:         " << fps << std::endl;
  std::cout << "Initial Vel: " << initial_velocity << " m/s" << std::endl;
  std::cout << "---------------------------------------------------------"
            << std::endl;

  // Initialize the high-level pipeline scheduler
  cosmos_wam::PipelineScheduler scheduler(host, user, pass);

  if (host == "local") {
    return scheduler.runLocalOnly();
  } else {
    return scheduler.runEndToEnd(image_path, duration, fps, initial_velocity);
  }
}
