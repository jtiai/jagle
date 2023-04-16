
#include "antlr4-runtime.h"
#include "JagleLexer.h"
#include "JagleParser.h"

#include "visitor.h"

#include <catch2/catch_test_macros.hpp>

class VisitorTestsFixture {
public:
	VisitorTestsFixture(const std::string& inputStr) : input(inputStr), lexer(&input), tokens(&lexer), parser(&tokens) {
		// Set the error handler to BailErrorStrategy
		parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());
	}

	antlr4::ANTLRInputStream input;
	jagle::JagleLexer lexer;
	antlr4::CommonTokenStream tokens;
	jagle::JagleParser parser;
};

TEST_CASE("variable declaration, assign int", "[statement]") {
	const std::string inputStr = "a: int = 1";
	VisitorTestsFixture fixture(inputStr);

	GeneratingVisitor visitor;

	auto result = visitor.visit(fixture.parser.prog());
	std::string program = visitor.getStatements();

	REQUIRE(program == "int _jagle_a = 1;\n");
}

TEST_CASE("variable declaration, assign float", "[statement]") {
	const std::string inputStr = "b: float = 1.0";
	VisitorTestsFixture fixture(inputStr);

	GeneratingVisitor visitor;

	auto result = visitor.visit(fixture.parser.prog());
	std::string program = visitor.getStatements();

	REQUIRE(program == "float _jagle_b = 1.0;\n");
}

TEST_CASE("variable declaration, assign str", "[statement]") {
	const std::string inputStr = "c: str = \"xyzzy\"";
	VisitorTestsFixture fixture(inputStr);

	GeneratingVisitor visitor;

	auto result = visitor.visit(fixture.parser.prog());
	std::string program = visitor.getStatements();

	REQUIRE(program == "std::string _jagle_c = \"xyzzy\";\n");
}
