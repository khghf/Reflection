#pragma once
#ifdef ENABLE_PARSER
	#ifdef __clang__
		#define CLASS(...)__attribute__((annotate("reflect_class",__VA_ARGS__)))
		#define FUNCTION(...)__attribute__((annotate("reflect_function",__VA_ARGS__)))
		#define PROPERTY(...)__attribute__((annotate("reflect_property",__VA_ARGS__)))
		#define STRUCT(...)__attribute__((annotate("reflect_struct",__VA_ARGS__)))
		#define ENUM(...)__attribute__((annotate("reflect_enum",__VA_ARGS__)))
		#define REFLECT_BODY_COMBINE(x)REFLECT_BODY_##x()
		#define REFLECT_BODY_(x)REFLECT_BODY_COMBINE(x)
		#define REFLECT_BODY(...)REFLECT_BODY_(__LINE__)private:__attribute__((annotate("reflect_body",__VA_ARGS__)))const static bool _reflect_body_line_mark_=true;
	#else
		#define CLASS(...)
		#define FUNCTION(...)
		#define PROPERTY(...)
		#define REFLECT_BODY_(x,...)REFLECT_BODY_##x(...)
		#define REFLECT_BODY(...) REFLECT_BODY_(__LINE__,...)
	#endif // __clang__
#else
	#define CLASS(...)
	#define FUNCTION(...)
	#define PROPERTY(...)
	#define STRUCT(...)
	#define ENUM(...)
	#define REFLECT_BODY_COMBINE(x)REFLECT_BODY_##x()
	#define REFLECT_BODY_(x)REFLECT_BODY_COMBINE(x)
	#define REFLECT_BODY(...)REFLECT_BODY_(__LINE__)
#endif // ENABLE_PARSER

#ifdef REFLECT_PROJECT
#define Reflect_API _declspec(dllexport)
#else
#define Reflect_API _declspec(dllimport)
#endif // Reflect_API