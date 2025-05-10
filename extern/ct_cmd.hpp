/*
 * MIT License

 * Copyright (c) 2025 FerociousMagikarp

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <array>
#include <algorithm>
#include <concepts>
#include <type_traits>
#include <string>
#include <string_view>
#include <source_location>
#include <tuple>
#include <exception>
#include <format>
#include <optional>
#include <charconv>
#include <vector>

namespace ct_cmd
{

namespace detail
{

template <typename T>
	requires std::same_as<T, std::decay_t<T>>
consteval auto get_t_name() noexcept
{
	std::string_view res(std::source_location::current().function_name());
#if defined(__clang__)
	auto pos_start = res.rfind('[') + sizeof("T = ");
	auto pos_end = res.rfind(']');
	res = res.substr(pos_start, pos_end - pos_start);
#elif defined(__GNUC__)
	auto pos_start = res.rfind('[') + sizeof("with T = ");
	auto pos_end = res.rfind(']');
	res = res.substr(pos_start, pos_end - pos_start);
#elif defined(_MSC_VER)
	auto pos_start = res.rfind('<');
	auto pos_end = res.rfind('>');
	res = res.substr(pos_start + 1, pos_end - pos_start - 1);
	if (auto pos = res.rfind("struct "); pos != std::string_view::npos)
		res = res.substr(pos + sizeof("struct ") - 1);
#else
	static_assert(false, "unsupported compiler");
#endif
	auto colon = res.rfind(':');
	if (colon != std::string_view::npos)
		res = res.substr(colon + 1);

	return res;
}

template <>
consteval auto get_t_name<std::string>() noexcept
{
	return std::string_view{"string"};
}

template <typename... T>
constexpr bool always_false = false;

template <std::size_t N, typename... Ts>
struct _find_T_at_N
{
	static_assert(N < sizeof...(Ts), "Out of range.");
	using type = std::decay_t<decltype(std::get<N>(std::declval<std::tuple<Ts...>>()))>;
};

template <std::size_t N, typename... Ts>
using find_T_at_N_t = typename _find_T_at_N<N, Ts...>::type;

struct null_value {};

template <typename P, typename = void>
struct _get_param_value
{
	using type = null_value;
};

template <typename T>
struct _get_param_value<T, std::void_t<typename T::value_type>>
{
	using _t = std::decay_t<typename T::value_type>;
	using type = std::conditional_t<std::is_void_v<_t>, null_value, _t>;
};

template <typename T>
using get_param_value_t = typename _get_param_value<T>::type;

template <typename... V>
struct _values;

template <typename... P>
struct _params
{
	using value_type = _values<get_param_value_t<P>...>;
};

struct help
{
	constexpr static char short_name = '?';
	constexpr static std::string_view message = "show help";
};

template <typename... P>
consteval auto get_param_names() noexcept
{
	return []<std::size_t... Idx>(std::index_sequence<Idx...>) ->std::array<std::string_view, sizeof...(P)>
	{
		return { get_t_name<P>()... };
	}(std::make_index_sequence<sizeof...(P)>());
}

template <typename P>
consteval char get_short_name() noexcept
{
	if constexpr (requires { {P::short_name} -> std::convertible_to<char>; })
		return P::short_name;
	else
		return '\0';
}

template <typename P>
consteval auto get_value_type_name() noexcept
{
	using value_type = get_param_value_t<P>;
	if constexpr (std::is_same_v<value_type, null_value>)
		return std::string_view{ "" };
	else
		return get_t_name<value_type>();
}

template <typename... P>
consteval auto get_param_short_names()
{
	return []<std::size_t... Idx>(std::index_sequence<Idx...>)->std::array<char, sizeof...(P)>
	{
		return { get_short_name<P>()...};
	}(std::make_index_sequence<sizeof...(P)>());
}

template <typename... V>
consteval auto get_no_value_flags()
{
	return []<std::size_t... Idx>(std::index_sequence<Idx...>)->std::array<bool, sizeof...(V)>
	{
		return { std::is_same_v<V, null_value>... };
	}(std::make_index_sequence<sizeof...(V)>());
}

template <typename T>
consteval bool is_must() noexcept
{
	if constexpr (requires { {T::must} -> std::convertible_to<bool>; })
		return T::must;
	else
		return false;
}

template <typename Param, std::size_t Idx, typename F>
	requires requires(F f) { {f(0)} -> std::convertible_to<bool>; }
bool _foreach_must(F&& f)
{
	if constexpr (detail::is_must<Param>())
		return f(Idx);
	else
		return true;
}

template <typename T>
consteval bool is_ignore_name() noexcept
{
	if constexpr (requires { {T::ignore_name} -> std::convertible_to<bool>; })
		return T::ignore_name;
	else
		return false;
}

template <typename... P>
consteval std::size_t get_ignore_name_id() noexcept
{
	auto res = []<std::size_t... Idx>(std::index_sequence<Idx...>)->std::size_t
	{
		return { ((is_ignore_name<P>() ? Idx : 0) + ...) };
	}(std::make_index_sequence<sizeof...(P)>());
	if (res == 0)
		return static_cast<std::size_t>(-1);
	return res;
}
template <typename T>
consteval bool has_default_value() noexcept
{
	using val_type = get_param_value_t<T>;
	if constexpr (std::is_same_v<val_type, null_value>)
		return false;
	using conv_type = std::conditional_t<std::is_same_v<val_type, std::string>, std::string_view, val_type>;
	if constexpr (requires { {T::default_value} -> std::convertible_to<conv_type>; })
		return true;
	else
		return false;
}

template <typename T>
consteval bool has_message() noexcept
{
	if constexpr (requires { {T::message} -> std::convertible_to<std::string_view>; })
		return true;
	else
		return false;
}

template <typename T>
inline const auto& get_default_value() noexcept
{
	static_assert(has_default_value<T>());
	return T::default_value;
}

template <typename P, typename... Params>
consteval std::size_t get_param_id()
{
	if constexpr (!(std::is_same_v<P, Params> || ...))
		static_assert(always_false<P>, "unknown param type");
	return []<std::size_t... Idx>(std::index_sequence<Idx...>) -> std::size_t
	{
		return ((std::is_same_v<P, Params> ? Idx : 0) + ...);
	} (std::make_index_sequence<sizeof...(Params)>());
}

template <typename T, typename... Ts>
consteval bool _check_type_or_shortname_same()
{
	constexpr std::size_t N = sizeof...(Ts);
	if constexpr (N == 0)
		return false;
	else
	{
		constexpr bool same = []<std::size_t... Idx>(std::index_sequence<Idx...>) -> bool
		{
			return ((std::is_same_v<T, Ts> || get_short_name<T>() == get_short_name<Ts>()) || ...);
		} (std::make_index_sequence<N>());
		if constexpr (same)
			return true;
		else
		{
			return _check_type_or_shortname_same<Ts...>();
		}
	}
}

template <typename... P>
consteval bool check()
{
	constexpr auto N = sizeof...(P);
	constexpr int ignore_name_count = []<std::size_t... Idx>(std::index_sequence<Idx...>) -> int
	{
		return ((is_ignore_name<P>() ? 1 : 0) + ...);
	} (std::make_index_sequence<N>());
	static_assert(ignore_name_count <= 1, "Ignore name argument count must less than 1.");

	static_assert(!_check_type_or_shortname_same<P...>(), "Duplicate name or short name.");

	return true;
}

} // namespace detail

class error : public std::exception
{
public:
	error(std::string message) : m_message(std::move(message)) {}
	virtual ~error() {}
	const char* what() const throw() override { return m_message.c_str(); }

private:
	std::string m_message;
};

template <typename T>
inline std::optional<T> from_string(const std::string_view str)
{
	static_assert(detail::always_false<T>, "Please overload this function of your own type.");
}

template <>
inline std::optional<std::string> from_string<std::string>(const std::string_view str)
{
	return std::string{str};
}

template <std::integral T>
inline std::optional<T> from_string(const std::string_view str)
{
	T value{};
	auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
	if (ec == std::errc())
		return value;
	return std::nullopt;
}

template <std::floating_point T>
inline std::optional<T> from_string(const std::string_view str)
{
	T value{};
	auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
	if (ec == std::errc())
		return value;
	return std::nullopt;
}

template <typename... T>
class parser;

template <typename... P, typename ...V>
class parser<detail::_params<P...>, detail::_values<V...>>
{
	static_assert(sizeof...(P) == sizeof...(V));

private:
	constexpr static std::size_t PARAM_COUNT = sizeof...(P);
	constexpr static std::size_t IGNORE_NAME_ID = detail::get_ignore_name_id<P...>();
	constexpr static auto PARAM_NAMES = detail::get_param_names<P...>();
	constexpr static auto SHORT_NAMES = detail::get_param_short_names<P...>();
	constexpr static auto NO_VALUE_FLAGS = detail::get_no_value_flags<V...>();

	template <typename F>
	bool foreach_must(F&& f)
	{
		return [f = std::forward<F>(f)] <std::size_t... Idx>(std::index_sequence<Idx...>) -> bool
		{
			return (detail::_foreach_must<P, Idx>(std::forward<const F>(f)) && ...);
		}(std::make_index_sequence<PARAM_COUNT>());
	}

	template <typename Param, typename Val, std::size_t Idx>
	void set_default_value()
	{
		if constexpr (detail::has_default_value<Param>())
		{
			if (!m_has_value[Idx])
			{
				std::get<Idx>(m_values) = detail::get_default_value<Param>();
				m_has_value[Idx] = true;
			}
		}
	}

	void foreach_set_default_value()
	{
		[this] <std::size_t... Idx>(std::index_sequence<Idx...>) -> void
		{
			(set_default_value<P, V, Idx>(), ...);
		}(std::make_index_sequence<PARAM_COUNT>());
	}

	bool _parse_value(std::size_t id, const std::string_view str)
	{
		if (id >= PARAM_COUNT)
		{
			m_error = std::format("Unknown argument option : {}.", str);
			return false;
		}
		if (!(this->*ASSIGNMENT_FUNCS[id])(str))
		{
			m_error = std::format("Analyze value failed : --{} = {}.", PARAM_NAMES[id], str);
			return false;
		}
		m_has_value[id] = true;
		return true;
	}

	std::tuple<V...> m_values{};
	std::array<bool, PARAM_COUNT> m_has_value{};
	std::string m_error = "";
	std::string m_program_name = "";

	template <std::size_t Id>
	bool set_value(const std::string_view str)
	{
		static_assert(Id < PARAM_COUNT);

		using val_type = detail::find_T_at_N_t<Id, V...>;
		if constexpr (std::is_same_v<val_type, detail::null_value>)
			return false;
		else
		{
			auto val = from_string<val_type>(str);
			if (!val.has_value())
				return false;
			std::get<Id>(m_values) = std::move(*val);
			return true;
		}
	}

	consteval static auto get_assignment_funcs()
	{
		return []<std::size_t... Idx>(std::index_sequence<Idx...>) -> std::array<bool (parser::*)(const std::string_view), PARAM_COUNT>
		{
			return { (&parser::set_value<Idx>)... };
		}(std::make_index_sequence<PARAM_COUNT>());
	}
	constexpr static auto ASSIGNMENT_FUNCS = get_assignment_funcs();

public:
	[[nodiscard]] bool parse(int argc, char* argv[])
	{
		m_has_value.fill(false);
		m_error = "";

		if (argc < 1)
		{
			m_error = "Argument count cannot be 0.";
			return false;
		}

		m_program_name = argv[0];
		std::size_t last_id = IGNORE_NAME_ID;
		for (int i = 1; i < argc; i++)
		{
			std::string_view arg{ argv[i] };
			if (arg.empty())
				continue;

			if (arg[0] != '-')
			{
				if (!_parse_value(last_id, arg))
					return false;
				last_id = IGNORE_NAME_ID;
			}
			else if (arg.size() > 1 && arg[1] == '-')
			{
				arg.remove_prefix(2);
				auto param_iter = std::find(PARAM_NAMES.begin(), PARAM_NAMES.end(), arg);
				if (param_iter == PARAM_NAMES.end())
				{
					m_error = std::format("Undefined option : --{}.", arg);
					return false;
				}
				last_id = static_cast<std::size_t>(std::distance(PARAM_NAMES.begin(), param_iter));
				if (NO_VALUE_FLAGS[last_id])
				{
					m_has_value[last_id] = true;
					last_id = IGNORE_NAME_ID;
				}
			}
			else
			{
				if (arg.size() != 2)
				{
					m_error = std::format("Error option short name : {}.", arg);
					return false;
				}
				auto param_iter = std::find(SHORT_NAMES.begin(), SHORT_NAMES.end(), arg.back());
				if (param_iter == SHORT_NAMES.end())
				{
					m_error = std::format("Undefined option short name : {}.", arg);
					return false;
				}
				last_id = static_cast<std::size_t>(std::distance(SHORT_NAMES.begin(), param_iter));
				if (NO_VALUE_FLAGS[last_id])
				{
					m_has_value[last_id] = true;
					last_id = IGNORE_NAME_ID;
				}
			}
		}

		foreach_set_default_value();

		bool must_check = foreach_must([this](std::size_t index) -> bool
			{
				if (!m_has_value[index])
				{
					m_error = std::format("Must set argument : --{}.", PARAM_NAMES[index]);
					return false;
				}
				return true;
			});
		if (!must_check)
			return false;

		return true;
	}

	template <typename T>
	[[nodiscard]] bool exist() const noexcept
	{
		if constexpr (detail::is_must<T>() || detail::has_default_value<T>())
			return true;
		constexpr std::size_t id = detail::get_param_id<T, P...>();
		return m_has_value[id];
	}

	template <typename T>
	[[nodiscard]] const auto& get() const
	{
		constexpr std::size_t id = detail::get_param_id<T, P...>();
		if constexpr (!detail::is_must<T>() && !detail::has_default_value<T>())
		{
			if (!m_has_value[id])
				throw error(std::format("Param \"{}\" doesn't have any value.", PARAM_NAMES[id]));
		}
		const auto& val = std::get<id>(m_values);
		if constexpr (std::is_same_v<std::decay_t<decltype(val)>, detail::null_value>)
			static_assert(detail::always_false<T>, "This param do not have a value type.");
		return val;
	}

	[[nodiscard]] const std::string_view get_error() const noexcept
	{
		return m_error;
	}

	[[nodiscard]] bool is_help() const noexcept
	{
		return m_has_value[0];
	}

	[[nodiscard]] std::string generate_help_message() const
	{
		std::string_view program_name{m_program_name};
		if (auto pos = program_name.rfind(".exe"); pos != std::string_view::npos)
			program_name = program_name.substr(0, pos);
		if (auto pos = program_name.rfind('\\'); pos != std::string_view::npos)
			program_name = program_name.substr(pos + 1);
		if (auto pos = program_name.rfind('/'); pos != std::string_view::npos)
			program_name = program_name.substr(pos + 1);

		std::vector<std::pair<std::string, std::string>> must_vec{};

		[&must_vec] <std::size_t... Idx>(std::index_sequence<Idx...>) -> void
		{
			auto func = [&must_vec]<std::size_t I>(std::integral_constant<std::size_t, I>) -> void
			{
				using p_type = detail::find_T_at_N_t<I, P...>;
				if constexpr (detail::is_must<p_type>() && !detail::has_default_value<p_type>())
				{
					constexpr std::string_view value_type_str = detail::get_value_type_name<p_type>();
					if constexpr (detail::is_ignore_name<p_type>())
						must_vec.push_back(std::make_pair(std::format("[--{}]", PARAM_NAMES[I]), std::string{ value_type_str }));
					else
						must_vec.push_back(std::make_pair(std::format("--{}", PARAM_NAMES[I]), std::string{ value_type_str }));
				}
			};
			(func(std::integral_constant<std::size_t, Idx>{}), ...);
		}(std::make_index_sequence<sizeof...(P)>());

		auto usage = std::format("usage: {} ", program_name);
		for (auto& [name, val] : must_vec)
		{
			if (!val.empty())
				usage += name + ' ' + val + ' ';
			else
				usage += name + ' ';
		}
		usage += "[options] ...\n";

		std::string options{ "options:\n" };
		std::vector<std::tuple<std::string_view, char, std::string_view, std::string_view, std::string>> detai_vec{};
		[&detai_vec] <std::size_t... Idx>(std::index_sequence<Idx...>) -> void
		{
			auto func = [&detai_vec]<std::size_t I>(std::integral_constant<std::size_t, I>) -> void
			{
				using p_type = detail::find_T_at_N_t<I, P...>;
				if constexpr (detail::has_message<p_type>())
				{
					constexpr std::string_view val_type_str = detail::get_value_type_name<p_type>();
					constexpr bool has_default_value = detail::has_default_value<p_type>();
					if constexpr (!has_default_value)
						detai_vec.push_back(std::make_tuple(PARAM_NAMES[I], SHORT_NAMES[I], std::string_view{ p_type::message }, val_type_str, std::string{ "" }));
					else
						detai_vec.push_back(std::make_tuple(PARAM_NAMES[I], SHORT_NAMES[I], std::string_view{ p_type::message }, val_type_str, std::format("{}", p_type::default_value)));
				}
			};
			(func(std::integral_constant<std::size_t, Idx>{}), ...);
		}(std::make_index_sequence<sizeof...(P)>());

		for (auto& [name, short_name, msg, value_type, default_value] : detai_vec)
		{
			if (short_name != '\0')
				options += std::format("  -{}, --{:<16} {} ", short_name, name, msg);
			else
				options += std::format("      --{:<16} {} ", name, msg);

			if (!value_type.empty())
			{
				if (!default_value.empty())
					options += std::format("({} [={}])", value_type, default_value);
				else
					options += std::format("({})", value_type);
			}

			options += '\n';
		}

		return usage + options;
	}
};

namespace detail
{

template <typename... T>
struct _make_parser
{
	constexpr static bool check_res = detail::check<T...>();
	using type = parser<detail::_params<T...>, typename detail::_params<T...>::value_type>;
};

template <typename... T>
using make_parser_t = typename _make_parser<T...>::type;

} // namespace detail

template <typename... T>
auto make_parser()
{
	return detail::make_parser_t<detail::help, T...>{};
}

template <typename... T>
auto make_parser_ptr()
{
	return std::make_unique<detail::make_parser_t<detail::help, T...>>();
}

} // namespace ct_cmd
