#include <iostream>
#include <filesystem>

#include <toml.hpp>
#include <fmt/core.h>

#include "CLI/CLI.hpp"

#include "antlr4-runtime.h"
#include "JagleLexer.h"
#include "JagleParser.h"

#include "visitor.h"

int main(int argc, const char* argv[]) {
	std::string source_fname;
	std::string target_fname;
	std::string config_fname = "jagle.toml";

	CLI::App app{ "Jagle transpiler to C++" };
	app.add_option("input file", source_fname, "Jagle source file to transpile")->required()->check(CLI::ExistingFile);
	app.add_option("target name", target_fname, "A target name to build")->required();
	app.add_option("-c,--config", config_fname, "A configuration file. Defaults to jagle.toml")->check(CLI::ExistingFile);

	CLI11_PARSE(app, argc, argv);

	std::string executable_fname(target_fname);
	target_fname += ".cpp";
	
	// Configuration
	auto config = toml::parse_file(config_fname);

	bool compiler_keep_source = config["compiler"]["keep_source"].value_or(false);
	std::string compiler_cmd(config["compiler"]["cmd"].value_or("g++"));
	std::string cmd = fmt::format(compiler_cmd, fmt::arg("source", target_fname), fmt::arg("target", executable_fname));
	
	std::ifstream stream;
	stream.open(source_fname);
	if (!stream.is_open()) {
		std::cout << "File '" << source_fname << "' does not exist!" << std::endl;
		return 1;
	}

	std::cout << "Parsing " << source_fname << " ..." << std::endl;
	std::cout << "Generating" << target_fname << std::endl;

	antlr4::ANTLRInputStream input(stream);
	jagle::JagleLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	jagle::JagleParser parser(&tokens);
	parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

	antlr4::tree::ParseTree* tree = parser.prog();

	GeneratingVisitor visitor;
	auto done = visitor.visit(tree);

	// Compile to exe
	if (done.has_value()) {
		visitor.writeOutput(target_fname);

		std::cout << "Compiling " << target_fname << " ..." << std::endl;
		std::ostringstream msgbuf;
		std::cout << cmd << std::endl;
		system(cmd.c_str());
	}
	return 0;
}
