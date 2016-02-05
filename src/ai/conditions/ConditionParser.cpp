#include "ConditionParser.h"
#include "conditions/Filter.h"
#include "AIRegistry.h"
#include <iostream>

namespace ai {

ConditionParser::ConditionParser(const IAIFactory& aiFactory, const std::string& conditionString) :
		IParser(), _aiFactory(aiFactory) {
	_conditionString = ai::Str::eraseAllSpaces(conditionString);
}

ConditionParser::~ConditionParser() {
}

void ConditionParser::splitConditions(const std::string& string, std::vector<std::string>& tokens) const {
	int inParameter = 0;
	int inChildren = 0;
	std::string token;
	for (std::string::const_iterator i = string.begin(); i != string.end(); ++i) {
		if (*i == '{')
			++inParameter;
		else if (*i == '}')
			--inParameter;
		else if (*i == '(')
			++inChildren;
		else if (*i == ')')
			--inChildren;

		if (inParameter == 0 && inChildren == 0) {
			if (*i == ',') {
				tokens.push_back(token);
				token.clear();
				continue;
			}
		}
		token.push_back(*i);
	}
	tokens.push_back(token);
}

bool ConditionParser::fillInnerConditions(ConditionFactoryContext& ctx, const std::string& conditionStr) {
	std::vector<std::string> conditions;
	splitConditions(conditionStr, conditions);
	if (conditions.size() > 1) {
		for (std::vector<std::string>::const_iterator i = conditions.begin(); i != conditions.end(); ++i) {
			if (!fillInnerConditions(ctx, *i))
				return false;
		}
	} else {
		std::string parameters;
		std::size_t n = conditionStr.find("(");
		if (n == std::string::npos || conditionStr.find("{") < n) {
			parameters = getBetween(conditionStr, "{", "}");
			n = conditionStr.find("{");
		}

		std::string name;
		if (n != std::string::npos) {
			name = conditionStr.substr(0, n);
		} else {
			name = conditionStr;
		}
		// filter condition is a little bit special and deserves some extra attention
		if (ctx.filter) {
			const FilterFactoryContext ctxInner(parameters);
			const FilterPtr& c = _aiFactory.createFilter(name, ctxInner);
			if (!c) {
				setError("could not create filter for " + name);
				return false;
			}
			ctx.filters.push_back(c);
		} else {
			ConditionFactoryContext ctxInner(parameters);
			ctxInner.filter = name == FILTER_NAME;
			n = conditionStr.find("(");
			if (n != std::string::npos) {
				const std::size_t r = conditionStr.rfind(")");
				if (r == std::string::npos) {
					setError("syntax error, missing closing brace");
					return false;
				}
				const std::string& inner = conditionStr.substr(n + 1, r - n - 1);
				if (!fillInnerConditions(ctxInner, inner))
					return false;
			}
			const ConditionPtr& c = _aiFactory.createCondition(name, ctxInner);
			if (!c) {
				setError("could not create inner condition for " + name);
				return false;
			}
			ctx.conditions.push_back(c);
		}
	}
	return true;
}

ConditionPtr ConditionParser::getCondition() {
	std::string parameters;
	std::size_t n = _conditionString.find("(");
	if (n == std::string::npos || _conditionString.find("{") < n) {
		parameters = getBetween(_conditionString, "{", "}");
		n = _conditionString.find("{");
	}
	std::string name;
	if (n != std::string::npos) {
		name = _conditionString.substr(0, n);
	} else {
		name = _conditionString;
	}
	ConditionFactoryContext ctx(parameters);
	ctx.filter = name == FILTER_NAME;
	n = _conditionString.find("(");
	if (n != std::string::npos) {
		const std::size_t r = _conditionString.rfind(")");
		if (r == std::string::npos) {
			setError("syntax error, missing closing brace");
			return ConditionPtr();
		}
		const std::string& inner = _conditionString.substr(n + 1, r - n - 1);
		if (!fillInnerConditions(ctx, inner)) {
			return ConditionPtr();
		}
	} else if (ctx.filter) {
		setError("missing details for Filter condition");
		return ConditionPtr();
	}
	const ConditionPtr& c = _aiFactory.createCondition(name, ctx);
	if (!c) {
		setError("could not create condition for " + name);
		return ConditionPtr();
	}
	return c;
}

}
