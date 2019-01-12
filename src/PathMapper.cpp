// PathMapper class
// Author: Ivan Blagopoluchnyy

#include "../src/AdjacencyObjectsGenerator.cpp"
#include "../src/GraphMapper.cpp"
#include "../src/GraphProcessor.cpp"
#include "../src/WSServer.cpp"


template <typename T, typename D>
std::ostream& operator<<(std::ostream& os, std::vector< std::pair <T, D> > &lst)
{
  for (const auto &p: lst)
  {
    os << p.first << ", " << p.second << "\n";
  }

  return os;
}


template <typename K, typename P, typename S>
std::ostream& operator<<(std::ostream& os, std::map <K, std::vector <std::pair <P, S>>> &mlst)
{
    for ( auto &m_iter: mlst)
    {
        std::cout << m_iter.first << " ";

        for (auto &m_node: m_iter.second)
        {
            std::cout << m_node.first << " " << m_node.second << "\n";
        }

    }

    return os;
}


int main()
{
    //use with already verifed port
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(25ms);

    WSServer(4560);

    return 0;
}
