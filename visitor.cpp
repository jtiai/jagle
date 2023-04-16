#include "visitor.h"

std::any GeneratingVisitor::visitProg(JP::ProgContext* ctx) {
	// Make the program in memory...
	if (!processStatements(ctx->stmtList(), statements)) {
		return std::any();
	}

	return true;
}

void GeneratingVisitor::writeOutput(const std::string& file_name) {
	OutputStream out;
	out.open(file_name);

	out << "#include \"jagle.hpp\"" << std::endl;
	out << std::endl;

	// Global data
	out << "// Global data" << std::endl;
	out << "int _jagle_data_idx = 0;" << std::endl;
	out << "std::variant<int, float, std::string> _jagle_data[] = {" << std::endl;
	out << getData() << std::endl;
	out << "};" << std::endl;
	out << std::endl;

	out << "// Function declarations" << std::endl;
	out << getFuncDecls() << std::endl;
	out << std::endl;

	out << "// Function declarations" << std::endl;
	out << getFuncBodies() << std::endl;
	out << std::endl;

	out << "// Main program" << std::endl;
	out << "int main(int argc, char* argv[]) {" << std::endl;


	out << getStatements() << std::endl;
	out << "std::cout << std::endl; // Temporary hack" << std::endl;  // Make sure that last line will cause (extra) linefeed
	out << std::endl << "return 0;" << std::endl;
	out << "}" << std::endl;

	// Output the data
	out.close();
}

std::any GeneratingVisitor::visitPrintStmt(JP::PrintStmtContext* ctx) {
	std::ostringstream oss_;
	oss_ << "std::cout";

	JP::PrintListContext* printListCtx = ctx->printList();

	for (const auto& child : printListCtx->children) {
		if (auto terminal_node = dynamic_cast<antlr4::tree::TerminalNode*>(child)) {
			auto token_type = terminal_node->getSymbol()->getType();
			if (token_type == jagle::JagleLexer::COMMA) {
				// Do nothing yet, comma is not supported
			}
		}
		else {
			oss_ << " << " << std::any_cast<std::string>(visit(child));
		}
	}


	if (auto last_node = dynamic_cast<antlr4::tree::TerminalNode*>(printListCtx->children.back())) {
		// Comma or semicolon at the end
		oss_ << ";" << std::endl;
	}
	else {
		// No comma or semicolon at the end
		oss_ << " << std::endl; " << std::endl;
	}

	return oss_.str();
}

std::any GeneratingVisitor::visitLiteral(JP::LiteralContext* ctx) {
	if (antlr4::tree::TerminalNode* string_literal = ctx->STRINGLITERAL()) {
		std::string quoted_str = string_literal->getText();
		return quoted_str;
	}
	if (antlr4::tree::TerminalNode* number_literal = ctx->NUMBER()) {
		std::string number_str = number_literal->getText();
		return number_str;
	}
	if (antlr4::tree::TerminalNode* float_literal = ctx->FLOAT()) {
		std::string float_str = float_literal->getText();
		return float_str;
	}

	return std::any();
}

std::any GeneratingVisitor::visitVariableDeclStmt(JP::VariableDeclStmtContext* ctx) {
	return fmt::format("{};\n", std::any_cast<std::string>(visit(ctx->variableDecl())));
}

std::any GeneratingVisitor::visitVariableDecl(JP::VariableDeclContext* ctx) {
	std::string var_type = std::any_cast<std::string>(visit(ctx->variableType()));

	std::string var_name = std::any_cast<std::string>(visit(ctx->identifier()));
	auto expr_result = visit(ctx->expression());

	if (expr_result.has_value()) {
		// Expression is actually resolvable
		std::string expr = std::any_cast<std::string>(expr_result);
		return fmt::format("{} {} = {}", var_type, var_name, expr);
	}
	else {
		// Nothing as a value
		return fmt::format("{} {}", var_type, var_name);

	}
}

std::any GeneratingVisitor::visitVariableAssignment(JP::VariableAssignmentContext* ctx) {
	std::string var_name = std::any_cast<std::string>(visit(ctx->identifier()));
	std::string expr = std::any_cast<std::string>(visit(ctx->expression()));

	return fmt::format("{} = {}", var_name, expr);
}

std::any GeneratingVisitor::visitIdentifier(JP::IdentifierContext* ctx) {
	auto var_name = ctx->getText();
	return makeIdentifier(var_name);
}

std::any GeneratingVisitor::visitForStmt(JP::ForStmtContext* ctx) {
	std::string var_name;
	std::string to_expr = std::any_cast<std::string>(visit(ctx->expression(0)));
	std::string step_expr;
	std::string variable_expr;

	if (const auto& variable_decl = ctx->variableDecl()) {
		var_name = std::any_cast<std::string>(visit(variable_decl->identifier()));
		variable_expr = std::any_cast<std::string>(visit(variable_decl));
	}
	if (const auto& variable_assign = ctx->variableAssignment()) {
		var_name = std::any_cast<std::string>(visit(variable_assign->identifier()));
		variable_expr = std::any_cast<std::string>(visit(variable_assign));
	}

	step_counter++;
	if (ctx->STEP()) {
		step_expr = std::any_cast<std::string>(visit(ctx->expression(1)));
	}
	else {
		step_expr = "1";
	}

	std::string statements = std::any_cast<std::string>(visit(ctx->stmtList()));
	std::string step_var_name = fmt::format("__jagle_step_{}", step_counter);

	std::string for_stmt = fmt::format(
		"auto {step_var_name} = {step_expr}; // internal\n"
		"if ({step_var_name} >= 0) {{\n"
		"// Positive stepping\n"
		"for ({variable_expr}; {var_name} <= {to_expr}; {var_name} += {step_var_name}) {{\n"
		"{statements}"
		"}}\n"
		"}}\n"
		"else {{\n"
		"// Negative stepping\n"
		"for ({variable_expr}; {var_name} >= {to_expr}; {var_name} += {step_var_name}) {{\n"
		"{statements}"
		"}}\n"
		"}}\n",
		fmt::arg("step_expr", step_expr),
		fmt::arg("variable_expr", variable_expr),
		fmt::arg("var_name", var_name),
		fmt::arg("to_expr", to_expr),
		fmt::arg("step_var_name", step_var_name),
		fmt::arg("statements", statements)
	);

	return for_stmt;
}

std::string GeneratingVisitor::getIdentifier(JP::VariableAssignmentContext* ctx) {
	auto identifier = ctx->identifier();
	return makeIdentifier(identifier->getText());
}

std::string GeneratingVisitor::getIdentifier(JP::IdentifierContext* ctx) {
	return makeIdentifier(ctx->getText());
}

std::string GeneratingVisitor::getFuncIdentifier(JP::IdentifierContext* ctx) {
	return "_func" + makeIdentifier(ctx->getText());
}

std::string GeneratingVisitor::getStatements() {
	return fmt::to_string(fmt::join(statements, ""));;
}

std::string GeneratingVisitor::getData() {
	return fmt::to_string(fmt::join(data, ", "));
}

std::string GeneratingVisitor::getFuncDecls() {
	return fmt::to_string(fmt::join(func_decls, "\n"));
}

std::string GeneratingVisitor::getFuncBodies() {
	return fmt::to_string(fmt::join(func_bodies, "\n"));
}

bool GeneratingVisitor::processStatements(std::vector<JP::StmtListContext*> stmtList, std::vector<std::string>& statements) {
	for (const auto& stmt : stmtList) {
		statements.push_back(std::any_cast<std::string>(visit(stmt)));
	}

	return true;
}

bool GeneratingVisitor::processStatements(JP::StmtListContext* stmtListCtx, std::vector<std::string>& statements) {
	for (const auto& stmt : stmtListCtx->statement()) {
		statements.push_back(std::any_cast<std::string>(visit(stmt)));
	}

	return true;
}

std::string GeneratingVisitor::makeIdentifier(const std::string& varName) {
	return fmt::format("_jagle_{}", varName);
}

std::any GeneratingVisitor::visitStmtList(JP::StmtListContext* ctx) {
	std::vector<std::string> statements;

	for (auto stmt : ctx->children) {
		auto res = visit(stmt);
		if (res.has_value()) {
			statements.push_back(fmt::format("{}", std::any_cast<std::string>(visit(stmt))));
		}
	}

	std::string result = fmt::to_string(fmt::join(statements, ""));
	return result;
}

// Math
std::any GeneratingVisitor::visitExponentExpression(JP::ExponentExpressionContext* ctx) {
	std::string lhs = std::any_cast<std::string>(visit(ctx->expression(0)));
	std::string rhs = std::any_cast<std::string>(visit(ctx->expression(1)));
	return fmt::format("std::pow({}, {})", lhs, rhs);
}

std::any GeneratingVisitor::visitMultiplyingExpression(JP::MultiplyingExpressionContext* ctx) {
	std::string lhs = std::any_cast<std::string>(visit(ctx->expression(0)));
	std::string rhs = std::any_cast<std::string>(visit(ctx->expression(1)));

	if (ctx->TIMES()) {
		return fmt::format("{} * {}", lhs, rhs);
	}
	if (ctx->DIV()) {
		return fmt::format("{} / {}", lhs, rhs);
	}
	if (ctx->MOD()) {
		return fmt::format("{} % {}", lhs, rhs);
	}
	return "visitMultiplyingExpression::ERROR!";
}

std::any GeneratingVisitor::visitAddingExpression(JP::AddingExpressionContext* ctx) {
	std::string lhs = std::any_cast<std::string>(visit(ctx->expression(0)));
	std::string rhs = std::any_cast<std::string>(visit(ctx->expression(1)));

	if (ctx->PLUS()) {
		return fmt::format("{} + {}", lhs, rhs);
	}
	if (ctx->MINUS()) {
		return fmt::format("{} - {}", lhs, rhs);
	}
	return "visitAddingExpression::ERROR!";
}

std::any GeneratingVisitor::visitUnaryExpression(JP::UnaryExpressionContext* ctx) {
	std::string expr = std::any_cast<std::string>(visit(ctx->expression()));

	if (ctx->NOT()) {
		return fmt::format("!{}", expr);
	}
	if (ctx->unary()->PLUS()) {
		return fmt::format("+({})", expr);
	}
	if (ctx->unary()->MINUS()) {
		return fmt::format("-({})", expr);
	}
	return "visitUnaryExpression::ERROR!";
}

std::any GeneratingVisitor::visitVariableType(JP::VariableTypeContext* ctx) {
	if (ctx->INT_TYPE()) {
		return std::string("int");
	}
	if (ctx->FLOAT_TYPE()) {
		return std::string("float");
	}
	if (ctx->STR_TYPE()) {
		return std::string("std::string");
	}

	return std::any();
}

std::any GeneratingVisitor::visitVariableAssignmentStmt(JP::VariableAssignmentStmtContext* ctx) {
	std::string expr = std::any_cast<std::string>(visit(ctx->variableAssignment()));
	return fmt::format("{};\n", std::any_cast<std::string>(visit(ctx->variableAssignment())));
}

std::any GeneratingVisitor::visitIfStmt(JP::IfStmtContext* ctx) {
	std::string expr = std::any_cast<std::string>(visit(ctx->expression()));
	std::string true_stmts = std::any_cast<std::string>(visit(ctx->stmtList(0)));

	if (!ctx->ELSE()) {
		return fmt::format("if ({}) {{\n{}}}\n", expr, true_stmts);
	}
	else {
		std::string false_stmts = std::any_cast<std::string>(visit(ctx->stmtList(1)));
		return fmt::format("if ({}) {{\n{}}}\n{}else {{\n{}}}\n", expr, true_stmts, false_stmts);
	}
	return "visitIfStmt::ERROR!";
}

std::any GeneratingVisitor::visitDataStmt(JP::DataStmtContext* ctx) {
	std::string unary = "";
	for (const auto& child : ctx->dataList()->children) {
		if (auto terminal_node = dynamic_cast<antlr4::tree::TerminalNode*>(child)) {
			auto token_type = terminal_node->getSymbol()->getType();
			if (token_type == jagle::JagleLexer::COMMA) {
				unary = "";
				continue;
			}
		}
		if (auto unary_node = dynamic_cast<JP::UnaryContext*>(child)) {
			unary = unary_node->getText();
		}
		if (auto literal_node = dynamic_cast<JP::LiteralContext*>(child)) {
			std::string literal = std::any_cast<std::string>(visit(literal_node));
			data.push_back(fmt::format("{}{}", unary, literal));
		}
	}
	return std::any();
}

std::any GeneratingVisitor::visitReadStmt(JP::ReadStmtContext* ctx) {
	return fmt::format("data_read({});\n", std::any_cast<std::string>(visit(ctx->identifier())));
}

std::any GeneratingVisitor::visitRestoreStmt(JP::RestoreStmtContext* ctx) {
	return "data_restore();\n";
}

std::any GeneratingVisitor::visitInputStmt(JP::InputStmtContext* ctx) {
	std::string prompt = "?";
	if (antlr4::Token* prompt_literal = ctx->prompt) {
		prompt = prompt_literal->getText();
	}

	std::string variableName = std::any_cast<std::string>(visit(ctx->identifier()));

	bool use_default = ctx->useDefault ? true : false;
	std::string default_value;

	if (auto default_expression = ctx->expression()) {
		default_value = std::any_cast<std::string>(visit(default_expression));
		return fmt::format("prompt_input({}, {}, {}, {});\n", prompt, variableName, use_default, default_value);
	}

	return fmt::format("prompt_input({}, {}, {});\n", prompt, variableName, false);

}

std::any GeneratingVisitor::visitRelationalExpression(JP::RelationalExpressionContext* ctx) {
	std::string lhs = std::any_cast<std::string>(visit(ctx->expression(0)));
	std::string rhs = std::any_cast<std::string>(visit(ctx->expression(1)));

	if (ctx->relop()->EQ()) {
		return fmt::format("{} == {}", lhs, rhs);
	}
	if (ctx->relop()->NEQ()) {
		return fmt::format("{} != {}", lhs, rhs);
	}
	if (ctx->relop()->LT()) {
		return fmt::format("{} < {}", lhs, rhs);
	}
	if (ctx->relop()->GT()) {
		return fmt::format("{} > {}", lhs, rhs);
	}
	if (ctx->relop()->LTE()) {
		return fmt::format("{} <= {}", lhs, rhs);
	}
	if (ctx->relop()->GTE()) {
		return fmt::format("{} >= {}", lhs, rhs);
	}
	return "visitRelationalExpression::ERROR!";
}

std::any GeneratingVisitor::visitLogicalExpression(JP::LogicalExpressionContext* ctx) {
	std::string lhs = std::any_cast<std::string>(visit(ctx->expression(0)));
	std::string rhs = std::any_cast<std::string>(visit(ctx->expression(1)));

	if (ctx->AND()) {
		return fmt::format("({} && {})", lhs, rhs);
	}
	if (ctx->OR()) {
		return fmt::format("({} || {})", lhs, rhs);
	}
	return "visitLogicalExpression::ERROR!";
}

std::any GeneratingVisitor::visitFuncCall(JP::FuncCallContext* ctx) {
	std::string id = getFuncIdentifier(ctx->identifier());
	std::string params;

	if (const auto& paramList = ctx->paramList()) {
		params = std::any_cast<std::string>(visit(paramList));
	}

	return fmt::format("{}({})", id, params);
}

std::any GeneratingVisitor::visitFuncDefStmt(JP::FuncDefStmtContext* ctx) {
	JP::FuncDefContext* f_ctx = ctx->funcDef();

	std::string funcIdentifier = getFuncIdentifier(f_ctx->identifier());
	std::string returnType;
	if (const auto& retType = f_ctx->variableType()) {
		auto result = visit(retType);
		returnType = std::any_cast<std::string>(visit(retType));
	}
	else {
		returnType = "void";
	}

	std::string arguments;
	if (f_ctx->argList()) {
		arguments = std::any_cast<std::string>(visit(f_ctx->argList()));
	}

	std::string funcDecl = fmt::format("{} {}({});", returnType, funcIdentifier, arguments);

	std::vector<std::string> stmts;
	processStatements(f_ctx->stmtList(), stmts);
	std::string stmtStr = fmt::to_string(fmt::join(stmts, ""));

	std::string funcBody = fmt::format("{} {}({}) {{\n{}}}\n", returnType, funcIdentifier, arguments, stmtStr);

	func_decls.push_back(funcDecl);
	func_bodies.push_back(funcBody);

	return std::any();
}

std::any GeneratingVisitor::visitFuncCallStmt(JP::FuncCallStmtContext* ctx) {
	std::string funcCall = std::any_cast<std::string>(visit(ctx->funcCall()));
	return fmt::format("{};\n", funcCall);
}

std::any GeneratingVisitor::visitValFunc(JP::ValFuncContext* ctx) {
	std::string expr = std::any_cast<std::string>(visit(ctx->expression()));
	return fmt::format("val({})", expr);
}

std::any GeneratingVisitor::visitArgList(JP::ArgListContext* ctx) {
	std::vector<std::string> args;
	for (auto it = ctx->children.begin(); it != ctx->children.end(); std::advance(it, 3)) {
		JP::IdentifierContext* argId = dynamic_cast<JP::IdentifierContext*>(*it);
		JP::VariableTypeContext* argType = dynamic_cast<JP::VariableTypeContext*>(*(it + 2));

		std::string id = getIdentifier(argId);
		std::string idType = std::any_cast<std::string>(visit(argType));
		args.push_back(fmt::format("{} {}", idType, id));
	}

	return fmt::to_string(fmt::join(args, ", "));
}

std::any GeneratingVisitor::visitParamList(JP::ParamListContext* ctx) {
	std::vector<std::string> expressions;

	for (auto& expr : ctx->expression()) {
		expressions.push_back(std::any_cast<std::string>(visit(expr)));
	}

	return fmt::to_string(fmt::join(expressions, ", "));
}

std::any GeneratingVisitor::visitReturnStmt(JP::ReturnStmtContext* ctx) {
	if (const auto& retExpr = ctx->expression()) {
		return fmt::format("return {};\n", std::any_cast<std::string>(visit(retExpr)));
	}
	else {
		return "return;\n";
	}
}
