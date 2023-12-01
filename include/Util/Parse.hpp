#pragma once

namespace Parse {
	template<typename T>
	inline T FetchIfPresent(YAML::Node a_node, T a_default) {
		return a_node.IsDefined() ? a_node.as<T>() : a_default;
	}
}