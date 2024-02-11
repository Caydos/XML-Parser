#ifndef TILED
#define TILED
#include <cstdint>
#include "XML.h"
#include "Common.h"

#define MAP_DIRECTORY "../Maps/"

namespace Tiled
{
	struct tile
	{
		//sf::Sprite* sprite;

		std::int64_t id;
		std::int64_t originalId;

		std::int64_t horizontalFlip;
		std::int64_t verticalFlip;
		std::int64_t diagonalFlip;
		std::int64_t hexagonal120Rotation;
	};
	struct image
	{
		bool lock;
		char* source;
		int width;// in pixels
		int height;// in pixels

		//sf::Texture* texture;

		image();
		~image();
	};
	struct TsxFile
	{
		char* name;
		int width;
		int height;
		int tilewidth;
		int tileheight;
		int tilecount;
		int columns;
	};
	struct tileset
	{
		image* images;
		unsigned int imageCount;

		bool lock;
		int firstgid;
		char* source;

		TsxFile tsxFile;

		tileset();
		~tileset();
	};
	struct data
	{
		bool lock;
		char* encoding;
		char* compression;

		tile* tiles;
		unsigned int tileCount;

		data();
		~data();
	};
	struct layer
	{
		bool lock;
		data data;
		int id;
		char* name;

		char* layerclass; /*class*/
		int x;
		int y;
		int width;
		int height;
		float opacity; /*0 to 1*/
		bool visible;
		int tintcolor;
		int offsetx;
		int offsety;
		int parallaxx;
		int parallaxy;


		layer();
		~layer();
	};
	struct Interaction
	{
		int id;
		int secondaryId;
		//sf::Vector2i position;
	};
	class Map
	{
	public:
		void LoadAttributes(XML::Tag* _tag);

		tileset* tilesets;
		unsigned int tilesetsCount;
		void AddTileset(XML::Tag* _tag);

		layer* layers;
		unsigned int layersCount;
		void AddLayer(XML::Tag* _tag);

		void SetAttributes(const char* _buffer, XML::Tag*& mapTag);

		std::vector<Interaction> interactions;
		Map();
		~Map();

#pragma region Attributes
		enum Orientation
		{
			Orthogonal,
			Isometric,
			Staggered,
			Hexagonal,
		};
		enum RenderOrder
		{
			Right_Down,
			Right_Up,
			Left_Down,
			Left_up,
		};
		float version;
		float tiledversion;
		Orientation orientation;
		RenderOrder renderorder;
		int compressionlevel; // default -1
		int width;
		int height;
		int tilewidth;
		int tileheight;

		int hexsidelength;
		int staggeraxis;
		int staggerindex;
		int parallaxoriginx;
		int parallaxoriginy;

		int backgroundcolor; //unk type

		int nextlayerid;
		int nextobjectid;
		bool infinite;
#pragma endregion
	};
	namespace Reader
	{
		namespace Flags
		{
			enum Flags
			{
				FLIPPED_HORIZONTALLY = 0x80000000,
				FLIPPED_VERTICALLY = 0x40000000,
				FLIPPED_DIAGONALLY = 0x20000000,
				ROTATED_HEXAGONAL_120 = 0x10000000,
			};
		}
		void Tmx(const char* _fileName, Map& _map);
		void UpdateTilesets(XML::Tag* _mapTag, Map& _map);
		void UpdateLayers(XML::Tag* _mapTag, Map& _map);
		void AddTileToGrid(tile& _tile, data& _data, unsigned int& _tileCount);
		void ReadCsvGrid(data& _data, const char* _buffer, unsigned int& _tileCount);
	}
}


#endif // !TILED