#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cassert>
#include <stdlib.h>

template <typename T>
struct TD;

struct NullType{};

template<typename H, typename xs>
struct TypeList;

// template <typename T> struct SizeOf
// {
//     enum{ value = sizeof(T)};
// };
// template <>
// struct SizeOf<NullType>
// {
//     enum{ value = 0};
// };

template <typename H, typename... Types> struct CreateTypeList;
template <typename H, typename... xs>
struct CreateTypeList
{
    using type = TypeList<H,typename CreateTypeList<xs...>::type>;
};
template <typename T>
struct CreateTypeList<T>
{
    using type = TypeList<T,NullType>;
};

template <typename T> struct Head;
template <typename H, typename xs>
struct Head<TypeList<H,xs>>
{
    using type = H;
};
template<>
struct Head<NullType>
{
    using type = NullType;
};
template <typename T> struct Tail;
template <typename H, typename xs>
struct Tail<TypeList<H,xs>>
{
    using type = xs;
};


template <typename T, uint8_t size, bool reject> struct RemoveFromListImpl;
template <typename H, typename xs,uint8_t size>
struct RemoveFromListImpl<TypeList<H,xs>,size,true>
{
    enum { reject = sizeof(typename Head<xs>::type) > size};
    using result = typename RemoveFromListImpl<xs, size, reject>::result;
};
template <typename H, typename xs,uint8_t size>
struct RemoveFromListImpl<TypeList<H,xs>,size,false>
{
    enum { reject = sizeof(typename Head<xs>::type) > size};
    using temp = typename RemoveFromListImpl<xs,size,reject>::result;
    using result = TypeList<H, temp>;
};
template<uint8_t size, bool reject>
struct RemoveFromListImpl<NullType,size,reject>
{
    using result = NullType;
};
template <typename T, uint8_t size> struct  RemoveFromList
{
    using result = typename RemoveFromListImpl<T,size, (sizeof(typename Head<T>::type) > size) >::result;
};

template <typename T> struct MaxStorageSize;
template <typename H, typename xs> 
struct MaxStorageSize<TypeList<H,xs>>
{
    enum{
        tailsize = MaxStorageSize<xs>::value,
        value = tailsize > sizeof(H) ? tailsize : sizeof(H)
    };
};

template <>
struct MaxStorageSize<NullType>
{
    enum{ value = 0};
};

// Takes a TypeList and a target size, and returns the type that's that size
template <typename T, size_t target_size, bool right_size> struct TypeWithSizeImpl;
template <size_t target_size, bool right_size>
struct TypeWithSizeImpl<NullType, target_size, right_size>
{
    using type = NullType;
};
template <typename H, typename xs,size_t target_size>
struct TypeWithSizeImpl<TypeList<H,xs>, target_size, true>
{
    using type = H;
};

template <typename H, typename xs, size_t target_size>
struct TypeWithSizeImpl<TypeList<H,xs>, target_size, false>
{
    using type = typename TypeWithSizeImpl<xs, target_size, sizeof(typename Head<xs>::type)==target_size>::type;
};

// Takes a TypeList and a target size, and returns the type that's that size
template <typename TL, size_t target_size> 
struct TypeWithSize{
    using type = typename TypeWithSizeImpl<TL,target_size, sizeof(typename Head<TL>::type)==target_size>::type;
}; 

// Takes a TypeList and return the largest type
template <typename TL> 
struct MaxSizeType{
    enum { max_size = MaxStorageSize<TL>::value };
    using type = typename TypeWithSize<TL,max_size>::type;
}; 

// this creates a union that has the head of a typelist
// along with and Align type of the next rest of the list
// so you get a cascading set of unions
template <typename T> struct AlignType;
template <typename H, typename xs>
struct AlignType<TypeList<H,xs>>
{
    union{
        H CurrentType;
        AlignType<xs> OtherTypes;
    };
};
template <>
struct AlignType<NullType>
{
};
// This struct makes a TypeList of all the basic types, struct's containing those types, pointer to the types,
// pointer to member function and pointer to member varaible
struct Alignment
{
    void foo();
    struct IntS{int foo;};
    struct BoolS{bool foo;};
    struct CharS{char foo;};
    struct WcharS{wchar_t foo;};
    struct FloatS{float foo;};
    struct DoubleS{double foo;};
    struct ShortIS{short int foo;};
    struct LongIS{long int foo;};
    struct LongLIS{long long int foo;};
    struct LongDS{long double foo;};
    struct PIntS{int* foo;};
    struct PBoolS{bool* foo;};
    struct PCharS{char* foo;};
    struct PWCharS{wchar_t* foo;};
    struct PFloatS{float* foo;};
    struct PDoubleS{double* foo;};
    struct PShortIS{short int* foo;};
    struct PLongIS{long int* foo;};
    struct PLongLS{long long int* foo;};
    struct PLongDS{long double* foo;};
    class VClass{
        virtual void vfoo(){};
    };
    using AlignmentTypes = CreateTypeList<int,bool,char,wchar_t,float,double,short int,long int,long long int,long double,
                                    int*,bool*,char*,wchar_t*,float*,double*,short int*,long int*,long long int*,long double*,
                                    void (*)(), void (Alignment::*)(), int Alignment::*,
                                    struct IntS,struct BoolS,struct CharS,struct WcharS,struct FloatS,struct DoubleS,
                                    struct ShortIS,struct LongIS,struct LongLIS,struct LongDS,struct PIntS,struct PBoolS,
                                    struct PCharS,struct PWCharS,struct PFloatS,struct PDoubleS,struct PShortIS,struct PLongIS,
                                    struct PLongLS,struct PLongDS,VClass
                                    >::type;
};

template <typename T>
struct CalcAlignmentTypes
{
    enum { max_size = MaxStorageSize<T>::value };
    using list = typename RemoveFromList<typename Alignment::AlignmentTypes,max_size>::result;
    using result = AlignType<list>;
};

// // allows you to call functions on an empty class that you give as a template argument
// template <typename Base, typename = typename std::enable_if<std::is_empty<Base>::value, Base>::type>
// Base& UseEmptyClass(){
//     return *(reinterpret_cast<Base*>(1));// This reinterpret cast is safe because we ensure the class
//                                          // is in fact empty, so we're not accessing any data, only functions
// }

// template <typename T, class U> struct DUSetVal;
// template <typename H, typename xs,typename U> 
// struct DUSetVal<TypeList<H,xs>,U> : public DUSetVal<xs,U>
// {
//     using DUSetVal<xs,U>::set; // makes parents set() visible to this. This happens up the inheritance chain
//                                // in effect making a family of generated functions that each take one of the
//                                // the types from the typelist
//     void set(H& val, U& obj)
//     {
//         new (obj.storage) H(val);
//     }
// };
// template<class U>
// struct DUSetVal<NullType, U>
// {
//     void set();
// };


template <typename T, typename TL> struct InTypeList;
template <typename T, typename H, typename xs>
struct InTypeList<T, TypeList<H,xs>>
{
    enum {value = std::is_same<T,H>::value || InTypeList<T,xs>::value}; //relies on shortcircuting
};
template <typename T>
struct InTypeList<T, NullType>
{
    enum {value = false};
};

template <typename T, typename TL, uint8_t index,bool same> struct IndexInTypeListImpl;
template <typename T, typename H,typename xs,uint8_t index>
struct IndexInTypeListImpl<T,TypeList<H,xs>,index,true>
{
    enum { value = index } ;
};
template <typename T, typename H,typename xs,uint8_t index>
struct IndexInTypeListImpl<T,TypeList<H,xs>,index,false>
{
    enum {value = IndexInTypeListImpl<T,xs,index+1,std::is_same<T,typename Head<xs>::type>::value>::value};
};
template <typename T, typename TL> struct IndexInTypeList;
template <typename T, typename H, typename xs>
struct IndexInTypeList<T, TypeList<H,xs>>
{
   enum {value = IndexInTypeListImpl<T,TypeList<H,xs>,0,std::is_same<T,H>::value>::value};
};

// template <typename TL, uint8_t index> struct GetAtTypeList;
// template <typename H, typename xs> 
// struct GetAtTypeList<TypeList<H,xs>, 0>
// {
//     using type = H;
// };
// template <typename H, typename xs, uint8_t index>
// struct GetAtTypeList<TypeList<H,xs>, index>
// {
//     using type = typename GetAtTypeList<xs, index-1>::type;
// };
// template <typename T, typename TL> struct Destruct{};
// template <typename H, typename xs, typename TL>
// struct Destruct<TypeList<H,xs>,TL> : public Destruct<xs,TL>
// {
//     template <typename DUT>
//     static void Destroy(uint8_t targetType, DUT& du)
//     {
//         uint8_t type = IndexInTypeList<H,TL>::value;
//         if(type == targetType){
//             H* ptr = reinterpret_cast<H*>(du.storage);
//             ptr->~H();
//         }else{
//             Destruct<xs,TL>::Destroy(targetType,du);
//         }
//     }
// };

template <typename T> struct Destruct{};
template <typename H, typename xs>
struct Destruct<TypeList<H,xs>> : public Destruct<xs>
{
    static void Destroy(uint8_t targetType, uint8_t* du_storage)
    {
        if(targetType==0){
            H* ptr = reinterpret_cast<H*>(du_storage);
            ptr->~H();
        }else{
            Destruct<xs>::Destroy(--targetType,du_storage);
        }
    }
};
template<>
struct Destruct<NullType>
{
    template<typename DUT>
    static void Destroy(uint8_t,DUT& du){}
};

template <typename CUR_TL> struct CopyFromStorage;
template <typename H, typename xs>
struct CopyFromStorage<TypeList<H,xs>> : public CopyFromStorage<xs>
{
    template <typename DUT>
    static void copy(const DUT& src, DUT& dest,uint8_t target_type)
    {
        // uint8_t type = IndexInTypeList<H,ORG_TL>::value;
        if(target_type == 0){
        // if(type == target_type){
            dest.set(src.template get<H>());
        }else{
            CopyFromStorage<xs>::copy(src,dest,--target_type);
        }
    }
};
template<>
struct CopyFromStorage<NullType> 
{
    template <typename DUT>
    static void copy(const DUT& src, DUT& dest,uint8_t target_type)
    {

    }
};

template <typename T> struct DoVisit;
template <typename H, typename xs>
struct DoVisit<TypeList<H,xs>> : public DoVisit<xs>
{
    template<typename V, typename Z, typename ReturnType = typename V::ReturnType>
    static ReturnType Go(V& visitor, const Z& du){
        uint8_t type = IndexInTypeList<H,typename Z::TList>::value;
        if(type == du.type()){
            return visitor.visit(du.template get<H>());
        }else{
            return DoVisit<xs>::Go(visitor,du);
        }
    }
};

template<>
struct DoVisit<NullType>
{
    template<typename V, typename Z, typename ReturnType = typename V::ReturnType>
    static ReturnType Go(V& visitor,const Z& du){
        std::cout << "Handle NullType" << std::endl;
        return ReturnType();
    }
};

// Takes a vistor class and a typelist and verifies that the visitor class
// declares a visit function for each type in the typelist
template <typename V, typename T> struct HasFuncs;
template <typename V, typename H, typename xs>
struct HasFuncs<V, TypeList<H,xs>>
{
    // the trick here is that template parameters can be types, integral constants or pointers. 
    template <typename U, U u>
    struct Has
    {
        enum { value = 1};
    };
    // FuncSig is a pointer-to-member-function that has the same signature as the function of which we want
    // to verify its existence.. It's a pointer-to-member-function that has return type V::ReturnType and takes
    // and parameter of H type
    using FuncSig = typename V::ReturnType (V::*)(H&);
    // So in this step we call Has<> with FuncSig as a parameter and by takeing the address of a v/istor class's
    // visit function. This is just normal function pointer usage in C++.
    // If the visit functino exists, it's Has<> will be instantiated with U=FuncSing and u = address of a function
    // that has FuncSig as it's signature. If the function doesn't exist, you can't take it's address, and it
    // won't compile. We recurse down the type list as normal.
    enum { value = Has<FuncSig, &V::visit>::value && HasFuncs<V,xs>::value};
};
template <typename V>
struct HasFuncs<V, NullType>
{
    enum {value = true};
};

template<typename V, typename T, typename ReturnType=typename V::ReturnType>
ReturnType apply_visitor(V visitor, T& du)
{
    using TL = typename T::TList;
    static_assert(HasFuncs<V,TL>::value, "Visitor must handle all the types");
    return DoVisit<typename T::TList>::Go(visitor,du);
}

// class to derive visitors off of, you supply it the return type of the visit function
template <typename T>
struct Visitor
{
    using ReturnType = T;
};
template <typename T> struct GenericPrintVisitor;
template <typename H, typename xs> 
struct GenericPrintVisitor<TypeList<H,xs>> : public GenericPrintVisitor<xs>
{
    using GenericPrintVisitor<xs>::stream;
    GenericPrintVisitor(std::ostream& stream):GenericPrintVisitor<xs>(stream)
    {}
    // using ReturnType = void;
    using GenericPrintVisitor<xs>::visit;
    void visit(H& val){
        // std::cout << val;
        stream << val;
    }
};
template<>
struct GenericPrintVisitor<NullType> : public Visitor<void>
{
    std::ostream& stream;
    GenericPrintVisitor(std::ostream& stream):stream(stream)
    {}
    void visit(NullType& );
    using ReturnType = void;
};

template <typename DU, typename T,typename ORG_TL> struct ConversionCasts;
template <typename DU, typename H, typename xs, typename ORG_TL>
struct ConversionCasts<DU, TypeList<H,xs>, ORG_TL> : ConversionCasts<DU,xs, ORG_TL>
{
    using ConversionCasts<DU,xs,ORG_TL>::du;
    ConversionCasts(DU* du):ConversionCasts<DU,xs, ORG_TL>(du){

    }
    H getter(uint8_t type){
        if(type == 0){
            return du->template get<H>();
        }else{
            return getter(--type);
        }
    }
    operator H() 
    {
        uint8_t type = IndexInTypeList<H, ORG_TL>::value;
        if(du->type() != type){
            throw "invalid conversion attempt";
        }
        return getter(type);
    }
};
template <typename DU, typename ORG_TL>
struct ConversionCasts<DU, NullType, ORG_TL>
{
    ConversionCasts(DU* du):du(du){};
    DU* du;
};

template<typename TL, typename Align = typename CalcAlignmentTypes<TL>::result>// class DUnion;
class DUnion : public ConversionCasts<DUnion<TL>, TL, TL>
{
public:
    using TList = TL;
    template <typename T, typename = typename std::enable_if<InTypeList<T,TList>::value>::type>
    DUnion (T&& val):ConversionCasts<DUnion<TList>, TList, TList>(this),moved(false)
    {
        set(val);
    }
    DUnion (const DUnion<TList>& obj):ConversionCasts<DUnion<TList>,TList,TList >(this),moved(false)
    {
        type_ = obj.type_;
        copy = obj.copy;
        copy(obj,*this);
    }
    DUnion<TList>& operator=(DUnion<TList>& obj)
    {
        type_ = obj.type_;
        copy = obj.copy;
        copy(obj,*this);
        return *this;
    }
    // // template <typename T, typename = typename std::enable_if<!std::is_base_of<std::string, T>::value>::type>
    // // this way of moving will not work with std::string due to the way it determines whethere or not it's using
    // // small string optimization. If you are and then move it, you'll get a double free and crash the program.
    // // This is likely to be an uncommon problem, so we could explicitly test here if it's a string and then copy it.
    // // But that requires as many function calls as there are types in the typelist, negating some, maybe all, of the benefit of moving.
    // // Moving isn't very fast anyways, since it requires copying a buffer, it only saves you the cost of constructing an object.
    // // So the best move is probably to just disable moving and rely only on copying. Or, instead of the DUnion keeping a statically
    // // allocated buffer, if it has a dynamically allocated instead, you could cheaply move it and this issue with string
    // // would go away.
    // DUnion (DUnion<TList>&& obj):ConversionCasts<DUnion<TList>, TList, TList>(this)
    // {
    //     std::cout << "moving" << std::endl;
    //     type_ = obj.type_;
    //     copy = obj.copy;
    //     // std::copy(obj.storage,obj.storage+storage_size,storage);
    //     std::memcpy(storage,obj.storage,storage_size);
        
    //     obj.moved = true;
    // }
    // DUnion<TList>& operator=(DUnion<TList>&& obj)//:ConversionCasts<DUnion<TypeList<H,xs>>, TypeList<H,xs>, TypeList<H,xs>>
    // {
    //     std::cout << "move assigning " << std::endl;
    //     type_ = obj.type_;
    //     copy = obj.copy;
    //     std::copy(obj.storage,obj.storage+storage_size,storage);
    //     obj.moved = true;
    //     return *this;
    // }
    friend auto operator <<(std::ostream& os, const DUnion& obj) -> std::ostream&
    {
        GenericPrintVisitor<TList> gp(os);
        DoVisit<TList>::Go(gp,obj);
        return os;
    }

    ~DUnion()
    {
        if(!moved){
            Destruct<TList>::Destroy(type_,this->storage);
        }
        moved = true;
    }
    template <typename T, typename = typename std::enable_if<InTypeList<T,TList>::value>::type>
    void set(const T& val){
        type_ = IndexInTypeList<T,TList>::value;
        copy = &CopyStorage<T>::copy;

        new (storage) T(val); // construct an object of type T, passing val to it's constructor
    }
    template <typename T, typename = typename std::enable_if<InTypeList<T,TList>::value>::type>
    void set(const T&& val){
        type_ = IndexInTypeList<T,TList>::value;
        copy = &CopyStorage<T>::copy;

        new (storage) T(val); // construct an object of type T, passing val to it's constructor
                              // and construct it in the space pointed by storage, using the fact
                              // array names decay to pointers
    }
    template <typename Z, typename = typename std::enable_if<InTypeList<Z,TList>::value>::type>
    Z& get() const {
        return *(reinterpret_cast<Z*>(const_cast<uint8_t*>(storage)));
    }
    inline uint8_t type() const { return type_; }
private:
    // template <typename T>
    // struct Destroy
    // {
    //     static void destroy(uint8_t* storage)
    //     {
    //         T* ptr = reinterpret_cast<T*>(storage);
    //         ptr->~T();
    //     }
    // };

    template <typename T>
    struct CopyStorage
    {
        static void copy(const DUnion& src, DUnion& dest)
        {
            dest.set(src.template get<T>());
        }
    };
    // template <typename T>
    // struct Print
    // {
    //     static void print(const DUnion& du, std::ostream& stream)
    //     {
    //         // stream << du.template get<T>();
    //         std::cout << "##";
    //         stream << du.get<T>();
    //     }
    // };
    // Takes up and extra 8 bytes in the struct, but we want copying to be faster. The other way has more function calls
    // but doesn't require a function pointer
    using CopyFunc = void (*)(const DUnion&, DUnion&);
    CopyFunc copy;
    // using PrintFunc = void (*)(const DUnion&, std::ostream& stream);
    // PrintFunc print;
    constexpr static size_t storage_size = MaxStorageSize<TList>::value;
    union{
        uint8_t storage[storage_size];
        Align dummy_;
    };
    uint8_t type_;
    bool moved;
};

struct my_str : public std::string
{
    static int cnt;
    my_str() : std::string()
    {
        num = cnt++;
        std::cout << "my_str constructor empty" << num << std::endl;
    }
    my_str(const char str[]) : std::string(str)
    {
        num = cnt++;
        std::cout << "my_str constructor " << num << std::endl;
    }
    my_str(const my_str& obj) : std::string(obj)
    {
        num = cnt++;
        std::cout << "my_str copy constructor " << num << std::endl;
    }
    my_str(my_str&& obj) : std::string(std::move(obj))
    {
        num = cnt++;
        std::cout << "my_str move constructor " << num << std::endl;
    }
    my_str& operator=(my_str&& obj)
    {
        std::cout << "move assigning " << std::endl;
        return *this;
    }
    ~my_str(){
        std::cout << "my str destructor " << num <<  std::endl;
    }
    int num;
};
int my_str::cnt = 0;
struct PrintVisitor : public Visitor<void> // void is the return type of the visit funcs
{
    void visit(double& val)
    {
        std::cout << "From visitor: " << val << std::endl;
    }
    void visit(char& val)
    {
        std::cout << "From visitor: " << val << std::endl;
    }
    void visit(std::string& val)
    {
        std::cout << "From visitor: " << val << std::endl;
    }
    void visit(my_str& val)
    {
        std::cout << "From visitor: " << val << std::endl;
    }
};


// struct CalcVisitor : public Visitor<int>
// {
//     int visit(double val)
//     {
//         std::cout << "From visitor: " << val << std::endl;
//         // return (int)val;
//         return 69;
//     }
//     int visit(char val)
//     {
//         std::cout << "From visitor: " << val << std::endl;
//         return (int)val;
//     }
//     int visit(std::string val)
//     {
//         std::cout << "From visitor: " << val << std::endl;
//         return 255;
//     }
// };
// struct AddVisitor : public Visitor<int> // int is the return type of the visit funcs
// {
//     uint32_t sum=0;
//     int visit(double val)
//     {
//         sum += val;
//         std::cout << "From visitor: " << val << std::endl;
//         return sum;
//     }
//     int visit(char val)
//     {
//         sum += val;
//         std::cout << "From visitor: " << val << std::endl;
//         return sum;
//     }
//     int visit(std::string val)
//     {
//         uint32_t tsum = 0;
//         for(const auto& c: val){
//             tsum += c;
//         }
//         std::cout << "From visitor: " << val << std::endl;
//         return sum+tsum;
//     }
// };


struct MyStruct
{
    static int i;
    MyStruct()
    {
        val = i++;
    }
    MyStruct(const MyStruct &obj)
    {
        // val = obj.val;
        val = i++;
    }
    ~MyStruct()
    {
    }
    int val;
};
int MyStruct::i = 0;

int main()
{
    using TL = CreateTypeList<char, double, std::string>::type;
    DUnion<TL> du(std::string("Hi World"));
    DUnion<TL> du2(3.14);

    std::vector<DUnion<TL>> vect;
    vect.push_back(DUnion<TL>(std::string("Hello World2")));
    vect.push_back(DUnion<TL>('a'));
    vect.push_back(du); 
    vect.push_back(du2); 

    // // // // CalcVisitor cv;
    // // // // AddVisitor av;
    // // // // apply_visitor(PrintVisitor(),du3);
    for(const auto& item : vect){
        apply_visitor(PrintVisitor(),item);
    }
    return 0;
}