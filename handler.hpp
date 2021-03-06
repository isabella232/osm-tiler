#include <osmium/handler.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/item_type.hpp>
#include <string>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

using namespace rapidjson;
using namespace std;

namespace fs = boost::filesystem;

static const auto decimal_to_radian = M_PI / 180;

struct Tile {
  uint x;
  uint y;
  uint z;
};

class Handler : public osmium::handler::Handler {
  const uint z;
  const string output;
  unordered_map<int,unordered_set<string>> indices;

  public:
    Handler(const uint tileZ, const string & outputFlag) :
      z(tileZ),
      output(outputFlag) {}

    Tile pointToTile(const double lon, const double lat) {
      auto tile = Tile();

      auto latSin = std::sin(lat * decimal_to_radian);
      auto z2 = std::pow(2, z);
      tile.x = std::floor(z2 * (lon / 360 + 0.5));
      tile.y = std::floor(z2 * (0.5 - 0.25 * std::log((1 + latSin) / (1 - latSin)) / M_PI));
      tile.z = z;

      return tile;
    }

    string xy (const Tile & tile) {
      return to_string(tile.x) + "/" + to_string(tile.y);
    }

    void mkdirTile(const Tile & tile) {
      std::ostringstream s;
      s << output << "/" << tile.x << "/" << tile.y;
      fs::create_directories(s.str());
    }

    void appendData(const string & tilePath, const string & data) {
      std::ostringstream path;
      path << output << "/" << tilePath << "/data.json";
      ofstream myfile;
      myfile.open(path.str(), ios::app);
      myfile << data << endl;
      myfile.close();
    }

    void node(const osmium::Node & node) {
      const auto & tags = node.tags();
      auto lon = node.location().lon();
      auto lat = node.location().lat();
      auto id = node.id();

      Tile tile = pointToTile(lon, lat);
      mkdirTile(tile);
      unordered_set<string> tiles;
      tiles.insert(xy(tile));
      pair<int, unordered_set<string>> nodeIndex(id, tiles);

      indices.insert(nodeIndex);

      StringBuffer nodeBuffer;
      Writer<StringBuffer> nodeWriter(nodeBuffer);

      nodeWriter.StartObject();

      nodeWriter.Key("type");
      nodeWriter.String("node");
      nodeWriter.Key("id");
      nodeWriter.Int(id);
      nodeWriter.Key("lon");
      nodeWriter.Double(lon);
      nodeWriter.Key("lat");
      nodeWriter.Double(lat);
      nodeWriter.Key("version");
      nodeWriter.Int(node.version());
      nodeWriter.Key("timestamp");
      nodeWriter.Int(node.timestamp().seconds_since_epoch());
      nodeWriter.Key("user");
      nodeWriter.String(node.user());

      nodeWriter.Key("tags");
      nodeWriter.StartObject();
      for (const auto & tag : tags) {
        nodeWriter.Key(tag.key());
        nodeWriter.String(tag.value());
      }
      nodeWriter.EndObject();
      nodeWriter.EndObject();

      appendData(xy(tile), nodeBuffer.GetString());
    }

    void way(const osmium::Way & way) {
      const auto & tags = way.tags();
      const auto id = way.id();

      StringBuffer wayBuffer;
      Writer<StringBuffer> wayWriter(wayBuffer);

      wayWriter.StartObject();

      wayWriter.Key("type");
      wayWriter.String("way");
      wayWriter.Key("id");
      wayWriter.Int(id);
      wayWriter.Key("version");
      wayWriter.Int(way.version());
      wayWriter.Key("timestamp");
      wayWriter.Int(way.timestamp().seconds_since_epoch());
      wayWriter.Key("user");
      wayWriter.String(way.user());
      wayWriter.Key("closed");
      wayWriter.Bool(way.is_closed());

      wayWriter.Key("node_refs");
      wayWriter.StartArray();
      unordered_set<string> tiles;
      int tileCount = 0;
      for (const auto & node : way.nodes()) {
        int ref = node.ref();
        wayWriter.Int(ref);

        if(indices.count(ref)) {
          const auto & nodeTiles = indices.at(ref);
          
          for (auto & tile : nodeTiles) {
            tiles.insert(tile);
            tileCount++;
          }
        }
      }
      pair<int,unordered_set<string>> wayIndex(id, tiles);
      indices.insert(wayIndex);
      wayWriter.EndArray();

      wayWriter.Key("tags");
      wayWriter.StartObject();
      for (const auto & tag : tags) {
        wayWriter.Key(tag.key());
        wayWriter.String(tag.value());
      }
      wayWriter.EndObject();
      wayWriter.EndObject();

      for (const auto & tileXy : tiles) {
        appendData(tileXy, wayBuffer.GetString());
      }
    }

    void relation(const osmium::Relation & relation) {
      const auto & tags = relation.tags();
      const auto id = relation.id();

      StringBuffer relationBuffer;
      Writer<StringBuffer> relationWriter(relationBuffer);

      relationWriter.StartObject();

      relationWriter.Key("type");
      relationWriter.String("relation");
      relationWriter.Key("id");
      relationWriter.Int(id);
      relationWriter.Key("version");
      relationWriter.Int(relation.version());
      relationWriter.Key("timestamp");
      relationWriter.Int(relation.timestamp().seconds_since_epoch());
      relationWriter.Key("user");
      relationWriter.String(relation.user());

      relationWriter.Key("members");
      relationWriter.StartArray();
      unordered_set<string> tiles;
      int tileCount = 0;
      for (const auto & member : relation.members()) {
        const auto ref = member.ref();
        relationWriter.StartObject();
        relationWriter.Key("id");
        relationWriter.Int(ref);
        relationWriter.Key("type");
        relationWriter.String(item_type_to_name(member.type()));
        relationWriter.Key("role");
        relationWriter.String(member.role());
        relationWriter.EndObject();

        if(indices.count(ref)) {
          const auto & memberTiles = indices.at(ref);
          
          for (const auto & tile : memberTiles) {
            tiles.insert(tile);
            tileCount++;
          }
        }
      }
      pair<int,unordered_set<string>> relationIndex(id, tiles);
      indices.insert(relationIndex);
      relationWriter.EndArray();

      relationWriter.Key("tags");
      relationWriter.StartObject();
      for (const auto & tag : tags) {
        relationWriter.Key(tag.key());
        relationWriter.String(tag.value());
      }
      relationWriter.EndObject();
      relationWriter.EndObject();

      for (const auto & tileXy : tiles) {
        appendData(tileXy, relationBuffer.GetString());
      }
    }
};
