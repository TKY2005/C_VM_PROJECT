#include<stdexcept>

#include<Visitors/Visitors.hpp>

[[noreturn]] void NodeVisitor::undefinedNode()
{
    throw std::runtime_error("visitor does not support this parse node");
}
