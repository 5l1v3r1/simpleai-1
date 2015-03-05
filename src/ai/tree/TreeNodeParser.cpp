#include "TreeNodeParser.h"
#include "Steer.h"

namespace ai {

TreeNodeParser::TreeNodeParser(const IAIFactory& aiFactory, const std::string& taskString) :
		IParser(), _aiFactory(aiFactory) {
	_taskString = ai::Str::eraseAllSpaces(taskString);
}

TreeNodeParser::~TreeNodeParser() {
}

void TreeNodeParser::splitTasks(const std::string& string, std::vector<std::string>& tokens) const {
	bool inParameter = false;
	bool inChildren = false;
	std::string token;
	for (std::string::const_iterator i = string.begin(); i != string.end(); ++i) {
		if (*i == '{')
			inParameter = true;
		else if (*i == '}')
			inParameter = false;
		else if (*i == '(')
			inChildren = true;
		else if (*i == ')')
			inChildren = false;

		if (!inParameter && !inChildren) {
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

SteeringPtr TreeNodeParser::getSteering (const std::string& nodeName) {
	std::string steerType;
	const std::string& parameters = getBetween(nodeName, "{", "}");
	std::size_t n = nodeName.find("{");
	if (n == std::string::npos)
		n = nodeName.find("(");
	if (n != std::string::npos) {
		steerType = nodeName.substr(0, n);
	} else {
		steerType = nodeName;
	}

	return _aiFactory.createSteering(steerType, parameters);
}

TreeNodePtr TreeNodeParser::getTreeNode(const std::string& name) {
	std::string nodeType;
	std::string parameters;
	std::size_t n = _taskString.find("(");
	if (n == std::string::npos || _taskString.find("{") < n) {
		parameters = getBetween(_taskString, "{", "}");
		n = _taskString.find("{");
	}
	if (n != std::string::npos) {
		nodeType = _taskString.substr(0, n);
	} else {
		nodeType = _taskString;
	}
	const std::string& subTrees = getBetween(_taskString, "(", ")");
	if (!subTrees.empty()) {
		if (nodeType != "Steer")
			return TreeNodePtr();
		std::vector<std::string> tokens;
		splitTasks(subTrees, tokens);
		movement::Steerings steerings;
		for (const std::string& nodeName : tokens) {
			const SteeringPtr& steering = getSteering(nodeName);
			if (!steering)
				return TreeNodePtr();
			steerings.push_back(steering);
		}
		const SteerNodeFactoryContext steerFactoryCtx(name.empty() ? nodeType : name, parameters, True::get(), steerings);
		return _aiFactory.createSteerNode(nodeType, steerFactoryCtx);
	}

	const TreeNodeFactoryContext factoryCtx(name.empty() ? nodeType : name, parameters, True::get());
	return _aiFactory.createNode(nodeType, factoryCtx);
}
}
