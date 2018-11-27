#include "app.h"

int main(int argc, char **argv) {
  stubgenerator::App app;
  return app.runStubGenerator(argc, argv);
}
