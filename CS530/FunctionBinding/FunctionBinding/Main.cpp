#include <stdio.h>
#include <vector>
#include <map>
#include <string>


typedef void (*SimpleFunction)(void);

void TakeFunction(SimpleFunction p)
{
	(*p)();
}

struct Variant
{
	union
	{
		int IntValue;
		float FloatValue;
		double Value;
		void* Pointer;
	};

	template<typename type>
	Variant(const type& value)
	{
		As<type>() = value;
	}

	template<typename type>
	type& As()
	{
		return *(type*)&IntValue;
	}
};


typedef  std::vector<Variant> ArgumentVector;
typedef void* Object;

typedef void (*VariantFunction)(Object object, ArgumentVector& args);


template< void (*FunctionAddress)(void) >
void Invoker(Object object, ArgumentVector& args)
{
	(*FunctionAddress)();
}

template<typename ClassType, typename param0, void (ClassType::*MethodPtr)(param0) >
void InvokerClass(Object object, ArgumentVector& args)
{
	ClassType* instance = (ClassType*)object;
	(instance->*MethodPtr)(args[0].As<param0>());
}

class FunctionObject
{
public:
	virtual void Invoke(Object object, ArgumentVector& args)=0;
};

class VoidFunction : public FunctionObject
{
public:
	SimpleFunction Function;
	VoidFunction(SimpleFunction f)
		:Function(f)
	{}

	void Invoke(Object object, ArgumentVector& args) override
	{
		(*Function)();
	}
};


template<typename ClassType, typename Param0Type>
class ClassFunctionOne : public FunctionObject
{
public:
	typedef void (ClassType::*MyFunctionType)(Param0Type);

	MyFunctionType Function;
	ClassFunctionOne(MyFunctionType f)
		:Function(f)
	{}

	void Invoke(Object object, ArgumentVector& args) override
	{
		ClassType* instance = (ClassType*)object;
		(instance->*Function)(args[0].As<Param0Type>());
	}
};



template<typename Param0Type>
class VoidFunctionOne : public FunctionObject
{
public:
	typedef void (*MyFunctionType)(Param0Type);

	MyFunctionType Function;
	VoidFunctionOne(MyFunctionType f)
		:Function(f)
	{}

	void Invoke(Object object, ArgumentVector& args) override
	{
		(*Function)(args[0].As<Param0Type>());
	}
};


void AFunction()
{
	printf("AFunction Hello\n");
}

void AFunctionFloat(float f)
{
	printf("AFunction Hello %g\n", f);
}

class MetaType
{
public:
	std::map<std::string, FunctionObject*> Functions;
};





template<typename type>
void BindMeta()
{
	MetaType* metaType = new MetaType();
	type::BindMeta(metaType);
	type::Meta = metaType;
}

//FunctionObject* Bind(function);

FunctionObject* Bind( void (*Func)() )
{
	return new VoidFunction(Func);
}

template<typename ParamType0>
FunctionObject* Bind( void (*Func)(ParamType0) )
{
	return new VoidFunctionOne<ParamType0>(Func);
}

template<typename ClassType, typename ParamType0>
FunctionObject* Bind( void (ClassType::*Func)(ParamType0) )
{
	return new ClassFunctionOne<ClassType, ParamType0>(Func);
}


template<typename TypeOfFunction, TypeOfFunction FunctionPointerA, 
	typename ClassType, typename ParamType0>
VariantFunction BindClassFunction( void (ClassType::*Func)(ParamType0) )
{
	return &InvokerClass<ClassType, ParamType0, FunctionPointerA>;
}


class SomeClass
{
public:
	typedef SomeClass this_type;
	static MetaType* Meta;
	static void BindMeta(MetaType* type);


	void AClassFunction(int a)
	{

	}
};


#define BindFunction(functionName) \
	type->Functions[#functionName] = Bind(&this_type::functionName);


//In Cpp
MetaType* SomeClass::Meta = NULL;
void SomeClass::BindMeta(MetaType* type)
{
	//BindBase(BaseClass);
	BindFunction(AClassFunction);

	//type->Functions["AClassFunction"] = Bind(&this_type::AClassFunction);
}

int main()
{
	//TakeFunction(AFunction);

	//Bind2(AFunctionFloat);

	BindClassFunction< decltype(&SomeClass::AClassFunction) , &SomeClass::AClassFunction>( &SomeClass::AClassFunction);

	//Bind2< decltype(AFunctionFloat), &AFunctionFloat >(AFunctionFloat);


	//VariantFunction a = &Invoker<AFunction>;

	//VariantFunction b = &Invoker<float, AFunctionFloat>;


	BindMeta<SomeClass>();

	FunctionObject* func1 = Bind(AFunction);
	
	FunctionObject* func2 = Bind(AFunctionFloat);
	
	FunctionObject* func3 = Bind(&SomeClass::AClassFunction);

	ArgumentVector v1;
	func1->Invoke(NULL, v1);

	ArgumentVector v2;
	v2.push_back( 0.5f );
	func2->Invoke(NULL, v2);

	printf("Done\n");

	return 0;
}