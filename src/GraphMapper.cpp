#ifndef GRAPHMAPPER_CPP
#define GRAPHMAPPER_CPP

#include <GraphMapper.hpp>

GraphMapper::GraphMapper(const graphPayload &&servers_data)
{
  //passed (or generated) servers names and it's weights of nodes assign to private member of class - table of pathes
  table_of_pathes = servers_data;

  //count of nodes it is a count of pairs
  nodes_count = servers_data.size();
}

void GraphMapper::get_shortest_path()
{

    /*--------------------------GRAPH SHORTEST PATH FIND START-----------------------*/

    lemon::SmartDigraph::ArcMap <double> costMap(ServersGraph);
    lemon::SmartDigraph::NodeMap <std::string> nodeMap(ServersGraph);

    //typedef std::unordered_map <int, std::vector <std::pair <std::string, std::string>>> graphPayload;
    std::map <std::string, int> nodes;

    unsigned long g_idx = 0;

    for (auto &it_table_data : table_of_pathes)
    {
        auto path_node = it_table_data.second;

        for (auto &s_node : path_node)
        {
            //minimal translation, because initial structure have only adjency data
            //not information about sequence of nodes as set
            if (nodes.count(s_node.first) == 0)
            {
                nodes.emplace(std::make_pair(s_node.first, g_idx));
            }
            
            if (nodes.count(s_node.second) == 0)
            {
                nodes.emplace(std::make_pair(s_node.second, g_idx));
            }
                                            
            ++g_idx;
        }
    }

    std::vector<Arc> arcs;
    for (auto &it_table_data : table_of_pathes)
    {
      auto path_node = it_table_data.second;

      //fill each edge (k) with first vertex (i) and second (i + 1)
      //and assign to the current edge weight, that was generated
      for (auto &s_node : path_node)
      {
          std::cout << "node: " << s_node.first << " - node: " << s_node.second << " with key = " << it_table_data.first << "\n";
          arcs.push_back(Arc{ s_node.first, s_node.second, static_cast<double>(it_table_data.first) });
      }
    }

    //defining the type of the Dijkstra Class
    using SptSolver = lemon::Dijkstra<lemon::SmartDigraph, lemon::SmartDigraph::ArcMap<double>>;

    //populate graph
    //nodes first
    lemon::SmartDigraph::Node currentNode;
    for (auto nodesIter = nodes.begin(); nodesIter != nodes.end(); ++nodesIter)
    {
        std::string key = nodesIter->first;
        currentNode = ServersGraph.addNode();
        nodeMap[currentNode] = key;
    }

    //then the arcs with the costs through the cost map
    lemon::SmartDigraph::Arc currentArc;
    for (auto arcsIter = arcs.begin(); arcsIter != arcs.end(); ++arcsIter)
    {
        int sourceIndex = nodes.at(arcsIter->sourceID);
        int targetIndex = nodes.at(arcsIter->targetID);

        lemon::SmartDigraph::Node sourceNode = ServersGraph.nodeFromId(sourceIndex);
        lemon::SmartDigraph::Node targetNode = ServersGraph.nodeFromId(targetIndex);

        currentArc = ServersGraph.addArc(sourceNode, targetNode);
        costMap[currentArc] = arcsIter->cost;
    }

    std::stringstream graph_stream;
    graph_stream << "digraph D {\n"
        << "  rankdir=LR\n"
        << "  size=\"4,3\"\n"
        << "  ratio=\"fill\"\n"
        << "  edge[dir=\"none\", style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";

    for (auto p = arcs.begin(); p != arcs.end(); p++)
    {
        graph_stream << p->sourceID << " -> " << p->targetID << "[label=\"" << p->cost << "\"";

        if (p->sourceID == p->targetID)
            graph_stream << ", color=\"black\"";
        else
            graph_stream << ", color=\"blue\"";

        graph_stream << "]";
    }

    graph_stream << "}";
    std::cout << "Final serialization in sstream: \n" << graph_stream.str() << "\n";

    const std::string start_point = nodes.begin()->first;
    const std::string target_point = nodes.rbegin()->first;

    //add source & target
    lemon::SmartDigraph::Node startN = ServersGraph.nodeFromId(nodes.at(start_point));
    lemon::SmartDigraph::Node endN = ServersGraph.nodeFromId(nodes.at(target_point));

    SptSolver spt(ServersGraph, costMap);
    spt.run(startN, endN);

    /* Walk in whole SPT is possible from specified orig and end
       but dest must be part of the SPT and
       an orig node must not be a dest node */
    std::vector<lemon::SmartDigraph::Node> path;
    path.reserve(nodes.size());
    path.resize(nodes.size());
    for (lemon::SmartDigraph::Node v = endN; v != startN; v = spt.predNode(v))
    {
        if (v != lemon::INVALID && spt.reached(v)) //special LEMON node constant
        {
            path.push_back(v);
        }
    }

    path.push_back(startN);

    double cost = spt.dist(endN);

    //print out the path with reverse iterator
    std::cout << "Shortest Path from " << "A" << " to " << "E" << " is: " << std::endl;
    for (auto p = path.rbegin(); p != path.rend(); ++p)
    {
        std::cout << nodeMap[*p] << " " << spt.dist(*p) << std::endl;
        std::cout << "|" << " " << " " << std::endl;
        std::cout << "v" << " " << " " << std::endl;
    }

    std::cout << "\nTotal cost for the shortest path is: " << cost << std::endl;

    graph_data = graph_stream.str();
}

const graphPayload GraphMapper::get_actual_table()
{
  return actual_pathes;
}

const std::string GraphMapper::get_graph()
{
  return graph_data;
}

#endif // GRAPHMAPPER_CPP
