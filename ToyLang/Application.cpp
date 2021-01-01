#define DEBUG_TREE true

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Debug.hpp"

#include "Scanner.h"
#include "Parser.h"
#include "Interpreter.h"

// TODO: return and struct or class

void run(const char* filePath)
{
    std::ifstream file(filePath);
    if (file.fail())
    {
        std::cout << "[ERROR] Unable to find file with path: " << filePath << std::endl;
        return;
    }

    std::stringstream sourceStream;
    sourceStream << file.rdbuf();
    std::string source = sourceStream.str();

    Scanner scanner(source);
    std::vector<Token> tokens = scanner.scanTokens();
    // for (auto& token : tokens)
    //     std::cout << token << std::endl;

    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> root = parser.parse();
    std::vector<Stmt*> root_ref;
    root_ref.reserve(root.capacity());
    
    for (auto& u_ptr : root)
        root_ref.push_back(u_ptr.get());

#if DEBUG_TREE
    AstDebugger astDebugger(root_ref);
    astDebugger.debug();
#endif

    Interpreter interpreter(root_ref);
    interpreter.run();
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "[ERROR] Invalid command line argument count." << std::endl;
        return 1;
    }

    run(argv[1]);
    return 0;
}