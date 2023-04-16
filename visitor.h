#pragma once

#include <filesystem>
#include <iostream>
#include <string_view>
#include <tuple>
#include <variant>

#include "fmt/core.h"
#include "fmt/format.h"

#include "antlr4-runtime.h"
#include "JagleLexer.h"
#include "JagleBaseVisitor.h"

using namespace jagle;
using JP = JagleParser;

class OutputStream {
private:
	std::ofstream of_;
	std::string file_name_;

public:
	OutputStream() = default;

	~OutputStream() {
		// Close open file.
		if (of_.is_open()) {
			of_.close();
		}
	}

	void open(const std::string_view file_name) {
		file_name_ = file_name;
		of_.open(std::filesystem::path(file_name));
	}

	void close() {
		of_.close();
	}

	OutputStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
		of_ << manip;
		std::cout << manip;
		return *this;
	}

	template <typename T>
	OutputStream& operator<<(const T& t) {
		of_ << t;
		std::cout << t;
		return *this;
	}
};

class GeneratingVisitor : public JagleBaseVisitor {
private:
	int step_counter = 0;

	std::vector<std::string> data;
	std::vector<std::string> func_decls;
	std::vector <std::string> func_bodies;
	std::vector<std::string> statements;

public:
	void writeOutput(const std::string& file_name);

	std::any visitProg(JP::ProgContext* ctx) override;
	std::any visitStmtList(JP::StmtListContext* ctx) override;

	// Statements
	std::any visitVariableDeclStmt(JP::VariableDeclStmtContext* ctx) override;
	std::any visitVariableAssignmentStmt(JP::VariableAssignmentStmtContext* ctx) override;
	std::any visitPrintStmt(JP::PrintStmtContext* ctx) override;
	std::any visitForStmt(JP::ForStmtContext* ctx) override;
	std::any visitIfStmt(JP::IfStmtContext* ctx) override;
	std::any visitDataStmt(JP::DataStmtContext* ctx) override;
	std::any visitReadStmt(JP::ReadStmtContext* ctx) override;
	std::any visitRestoreStmt(JP::RestoreStmtContext* ctx) override;
	std::any visitInputStmt(JP::InputStmtContext* ctx) override;

	// Expressions
	std::any visitLiteral(JP::LiteralContext* ctx) override;
	std::any visitVariableDecl(JP::VariableDeclContext* ctx) override;
	std::any visitVariableAssignment(JP::VariableAssignmentContext* ctx) override;
	std::any visitIdentifier(JP::IdentifierContext* ctx) override;
	std::any visitRelationalExpression(JP::RelationalExpressionContext* ctx) override;
	std::any visitLogicalExpression(JP::LogicalExpressionContext* ctx) override;

	// User defined functions
	std::any visitFuncDefStmt(JP::FuncDefStmtContext* ctx) override;
	std::any visitFuncCallStmt(JP::FuncCallStmtContext* ctx) override;
	std::any visitFuncCall(JP::FuncCallContext* ctx) override;
	std::any visitArgList(JP::ArgListContext* ctx) override;
	std::any visitParamList(JP::ParamListContext* ctx) override;
	std::any visitReturnStmt(JP::ReturnStmtContext* ctx) override;

	// Built-in functions
	std::any visitValFunc(JP::ValFuncContext* ctx) override;

	// Math
	std::any visitExponentExpression(JP::ExponentExpressionContext* ctx) override;
	std::any visitMultiplyingExpression(JP::MultiplyingExpressionContext* ctx) override;
	std::any visitAddingExpression(JP::AddingExpressionContext* ctx) override;
	std::any visitUnaryExpression(JP::UnaryExpressionContext* ctx) override;

	// Others
	std::any visitVariableType(JP::VariableTypeContext* ctx) override;

	// Helpers
	bool processStatements(std::vector<JP::StmtListContext*> ctx, std::vector<std::string>& statements);
	bool processStatements(JP::StmtListContext* ctx, std::vector<std::string>& statements);
	std::string makeIdentifier(const std::string& varName);
	std::string getIdentifier(JP::VariableAssignmentContext* ctx);
	std::string getIdentifier(JP::IdentifierContext* ctx);
	std::string getFuncIdentifier(JP::IdentifierContext* ctx);

	std::string getStatements();
	std::string getData();
	std::string getFuncDecls();
	std::string getFuncBodies();
};
