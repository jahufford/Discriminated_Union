This a type-safe discriminated union or variant implemementation in C++. 

Usage:

DUnion<char, int, std::string> du("hi"); //pass a list of types as the template parameter.
// Or we can create a typelist first with a using statement
using TL = CreateTypeList<char, int, std::string>
DUnion<TL> du2(5); // as long as the list of types is the same then du and d2 are the same type
// So now we can create a vector, and push our objects into it
// effectively createing a heterogenious collection
// This is useful where we can't modify the classes, or don't want to make them have a common base class.
// It's a form of type erasure.
std::vector<DUnion<TL>> vect;
vect.push_back(du);
vect.push_back(du2);

// we can interate the list and do operations using the visitor pattern
// see the code for the visit iterator
for(const auto& item : vect){
  apply_visitor(PrintVisitor(),item);
}






