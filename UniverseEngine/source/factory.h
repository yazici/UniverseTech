
#ifndef _FACTORY_HPP_
#define _FACTORY_HPP_

#include <map>
#include <functional>
using namespace std;

template <typename KeyType, typename ReturnType, template <typename ...ContainerArgs> class ContainerType = map>
class Factory {
public:
	using key_type = KeyType;
	using return_type = ReturnType;

	/* this class should not be instantiated */
	Factory(const Factory&) = delete;
	Factory(Factory&&) = delete;
	Factory() = delete;

	/* instantiating this allows to populate the associations in static environment */
	template <typename ...Args>
	class Initializer {
	private:
		using this_type = Initializer<Args...>;

		/* static accesor member to static local => make sure the map is always instantiated before being used */
		static ContainerType<KeyType, function<ReturnType(Args...)>>& getAssociations() {
			static ContainerType<KeyType, function<ReturnType(Args...)>> associations;
			return associations;
		}
	public:
		using key_type = KeyType;
		using return_type = ReturnType;

		Initializer(const KeyType& k, function<ReturnType(Args...)> f) {
			this_type::getAssociations()[k] = f;
		}

		static ReturnType create(KeyType&& k, Args... args) {
			return this_type::getAssociations()[k](args...);
		}
		static ReturnType create(const KeyType& k, Args... args) {
			return this_type::getAssociations()[k](args...);
		}

	};
};

#endif