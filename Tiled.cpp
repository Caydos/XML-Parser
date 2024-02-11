#include <iostream>
#include "Tiled.h"
#include "XML.h"


#pragma region Constructor/Destructors

Tiled::image::image()
{
	lock = false;
	source = nullptr;
	width = 0;
	height = 0;
	//texture = nullptr;
}

Tiled::image::~image()
{
	if (!lock && source != nullptr)
	{
		delete[] source;
	}
	//if (!lock && texture != nullptr)
	//{
	//	delete texture;
	//}
}

Tiled::tileset::tileset()
{
	images = nullptr;
	imageCount = 0;

	lock = false;
	firstgid = 0;
	source = nullptr;

	tsxFile.name = nullptr;
	tsxFile.columns = 0;
	tsxFile.tilecount = 0;
	tsxFile.height = 0;
	tsxFile.width = 0;
	tsxFile.tileheight = 0;
	tsxFile.tilewidth = 0;
}

Tiled::tileset::~tileset()
{
	if (!lock && source != nullptr)
	{
		delete[] source;
	}
	if (!lock && tsxFile.name != nullptr)
	{
		delete[] tsxFile.name;
	}
}

Tiled::data::data()
{
	lock = false;
	encoding = nullptr;
	compression = nullptr;
	tiles = nullptr;
	tileCount = 0;
}

Tiled::data::~data()
{
	if (!lock && encoding != nullptr)
	{
		delete[] encoding;
	}
	if (!lock && compression != nullptr)
	{
		delete[] compression;
	}
	if (!lock && tiles != nullptr)
	{
		delete[] tiles;
	}
}


Tiled::layer::layer()
{
	lock = false;
	data;
	id = 0;
	name = nullptr;

	layerclass = nullptr; /*class*/
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	opacity = 0; /*0 to 1*/
	visible = 0;
	tintcolor = 0;
	offsetx = 0;
	offsety = 0;
	parallaxx = 0;
	parallaxy = 0;
}



Tiled::layer::~layer()
{
	if (!lock && name != nullptr)
	{
		delete[] name;
	}
	if (!lock && layerclass != nullptr)
	{
		delete[] layerclass;
	}
}


Tiled::Map::Map()
{
	tilesets = nullptr;
	tilesetsCount = 0;
	layers = nullptr;
	layersCount = 0;
}

Tiled::Map::~Map()
{
	if (tilesets != nullptr)
	{
		for (unsigned int i = 0; i < tilesetsCount; i++)
		{
			tilesets[i].lock = false;
			for (unsigned int j = 0; j < tilesets[i].imageCount; j++)
			{
				tilesets[i].images[j].lock = false;
			}
		}
		delete[] tilesets;
	}
	if (layers != nullptr)
	{
		for (unsigned int i = 0; i < layersCount; i++)
		{
			layers[i].lock = false;
			layers[i].data.lock = false;
		}
		delete[] layers;
	}
}
#pragma endregion

void Tiled::Map::LoadAttributes(XML::Tag* _tag)
{
	const char* widthBff = XML::GetAttributeContent("width", _tag->attributesCount, _tag->attributes);
	this->width = (widthBff != nullptr) ? std::atoi(widthBff) : 0;

	const char* heightBff = XML::GetAttributeContent("height", _tag->attributesCount, _tag->attributes);
	this->height = (heightBff != nullptr) ? std::atoi(heightBff) : 0;

	const char* tilewidthBff = XML::GetAttributeContent("tilewidth", _tag->attributesCount, _tag->attributes);
	this->tilewidth = (tilewidthBff != nullptr) ? std::atoi(tilewidthBff) : 0;

	const char* tileheightBff = XML::GetAttributeContent("tileheight", _tag->attributesCount, _tag->attributes);
	this->tileheight = (tileheightBff != nullptr) ? std::atoi(tileheightBff) : 0;
}

void Tiled::Map::AddTileset(XML::Tag* _tag)
{
	if (tilesetsCount)
	{
		tileset* tempTileset = new tileset[tilesetsCount + 1];
		std::memcpy(tempTileset, tilesets, tilesetsCount * sizeof(tileset));
		delete[] tilesets;
		tilesets = tempTileset;
	}
	else
	{
		tilesets = new tileset[1];
	}
	tilesets[tilesetsCount].lock = true;

	const char* firstgidBff = XML::GetAttributeContent("firstgid", _tag->attributesCount, _tag->attributes);
	tilesets[tilesetsCount].firstgid = (firstgidBff != nullptr) ? std::atoi(firstgidBff) : 0;

	const char* sourceBff = XML::GetAttributeContent("source", _tag->attributesCount, _tag->attributes);
	if (sourceBff != nullptr)
	{
		size_t pathLength = strlen(sourceBff);
		tilesets[tilesetsCount].source = new char[pathLength + 1];
		std::memcpy(tilesets[tilesetsCount].source, sourceBff, pathLength);
		tilesets[tilesetsCount].source[pathLength] = '\0';

		size_t directoryLength = strlen(MAP_DIRECTORY);
		size_t fileNameLength = strlen(tilesets[tilesetsCount].source);
		char* path = new char[fileNameLength + directoryLength + 1];
		std::memcpy(path, MAP_DIRECTORY, directoryLength);
		std::memcpy(path + directoryLength, tilesets[tilesetsCount].source, fileNameLength);
		path[directoryLength + fileNameLength] = '\0';

		XML::File tsxFile = XML::ReadFile(path);
		delete[] path;

		for (size_t tagId = 0; tagId < tsxFile.tagCount; tagId++)
		{
			if (strcmp(tsxFile.tags[tagId].name, "tileset") == 0)
			{
				const char* nameBff = XML::GetAttributeContent("name", tsxFile.tags[tagId].attributesCount, tsxFile.tags[tagId].attributes);
				if (nameBff != nullptr)
				{
					size_t nameLength = strlen(nameBff);
					tilesets[tilesetsCount].tsxFile.name = new char[nameLength + 1];
					std::memcpy(tilesets[tilesetsCount].tsxFile.name, nameBff, nameLength);
					tilesets[tilesetsCount].tsxFile.name[nameLength] = '\0';
				}

				const char* tilewidthBff = XML::GetAttributeContent("tilewidth", tsxFile.tags[tagId].attributesCount, tsxFile.tags[tagId].attributes);
				tilesets[tilesetsCount].tsxFile.tilewidth = (tilewidthBff != nullptr) ? std::atoi(tilewidthBff) : 0;

				const char* tileheightBff = XML::GetAttributeContent("tileheight", tsxFile.tags[tagId].attributesCount, tsxFile.tags[tagId].attributes);
				tilesets[tilesetsCount].tsxFile.tileheight = (tileheightBff != nullptr) ? std::atoi(tileheightBff) : 0;

				const char* tilecountBff = XML::GetAttributeContent("tilecount", tsxFile.tags[tagId].attributesCount, tsxFile.tags[tagId].attributes);
				tilesets[tilesetsCount].tsxFile.tilecount = (tilecountBff != nullptr) ? std::atoi(tilecountBff) : 0;

				const char* columnsBff = XML::GetAttributeContent("columns", tsxFile.tags[tagId].attributesCount, tsxFile.tags[tagId].attributes);
				tilesets[tilesetsCount].tsxFile.columns = (columnsBff != nullptr) ? std::atoi(columnsBff) : 0;
				for (size_t secondaryTag = 0; secondaryTag < tsxFile.tags[tagId].tagCount; secondaryTag++)
				{
					if (strcmp(tsxFile.tags[tagId].tags[secondaryTag].name, "image") == 0)
					{
						XML::Tag* actualTag = &tsxFile.tags[tagId].tags[secondaryTag];

						if (!tilesets[tilesetsCount].imageCount && tilesets[tilesetsCount].images == nullptr)
						{
							tilesets[tilesetsCount].images = new image[1];
						}
						else
						{
							image* tempImages = new image[tilesets[tilesetsCount].imageCount + 1];
							std::memcpy(tempImages, tilesets[tilesetsCount].images, tilesets[tilesetsCount].imageCount * sizeof(image));
							delete[] tilesets[tilesetsCount].images;
							tilesets[tilesetsCount].images = tempImages;
						}

						image* actualImage = &tilesets[tilesetsCount].images[tilesets[tilesetsCount].imageCount];

						const char* sourceBff = XML::GetAttributeContent("source", actualTag->attributesCount, actualTag->attributes);
						if (sourceBff != nullptr)
						{
							size_t sourceLength = strlen(sourceBff);
							actualImage->source = new char[sourceLength + 1];
							std::memcpy(actualImage->source, sourceBff, sourceLength);
							actualImage->source[sourceLength] = '\0';
						}
						const char* widthBff = XML::GetAttributeContent("width", actualTag->attributesCount, actualTag->attributes);
						actualImage->width = (widthBff != nullptr) ? std::atoi(widthBff) : 0;

						const char* heightBff = XML::GetAttributeContent("height", actualTag->attributesCount, actualTag->attributes);
						actualImage->height = (heightBff != nullptr) ? std::atoi(heightBff) : 0;

						tilesets[tilesetsCount].imageCount++;
					}
				}
			}
		}
	}

	tilesetsCount++;
}

void Tiled::Map::AddLayer(XML::Tag* _tag)
{
	if (layersCount)
	{
		layer* tempLayers = new layer[layersCount + 1];
		std::memcpy(tempLayers, layers, layersCount * sizeof(layer));
		delete[] layers;
		layers = tempLayers;
	}
	else
	{
		layers = new layer[1];
	}
	layer* actualLayer = &layers[layersCount];
	actualLayer->lock = true;
	actualLayer->data.lock = true;
	// data
	for (unsigned int i = 0; i < _tag->tagCount; i++)
	{
		if (strcmp(_tag->tags[i].name, "data") == 0)
		{
			const char* encodingBff = XML::GetAttributeContent("encoding", _tag->tags[i].attributesCount, _tag->tags[i].attributes);
			if (encodingBff != nullptr)
			{
				size_t encodingLength = strlen(encodingBff);
				actualLayer->data.encoding = new char[encodingLength + 1];
				std::memcpy(actualLayer->data.encoding, encodingBff, encodingLength);
				actualLayer->data.encoding[encodingLength] = '\0';
			}
			const char* compressionBff = XML::GetAttributeContent("compression", _tag->tags[i].attributesCount, _tag->tags[i].attributes);
			if (compressionBff != nullptr)
			{
				size_t compressionLength = strlen(compressionBff);
				actualLayer->data.compression = new char[compressionLength + 1];
				std::memcpy(actualLayer->data.compression, compressionBff, compressionLength);
				actualLayer->data.compression[compressionLength] = '\0';
			}
			if (_tag->tags[i].content != nullptr && actualLayer->data.encoding != nullptr)
			{
				if (strcmp(actualLayer->data.encoding, "csv") == 0)
				{
					Reader::ReadCsvGrid(actualLayer->data, _tag->tags[i].content, actualLayer->data.tileCount);
				}
			}
		}
	}

	//int id;
	const char* idBff = XML::GetAttributeContent("id", _tag->attributesCount, _tag->attributes);
	actualLayer->id = (idBff != nullptr) ? std::atoi(idBff) : 0;

	//char* name;
	const char* nameBff = XML::GetAttributeContent("name", _tag->attributesCount, _tag->attributes);
	if (nameBff != nullptr)
	{
		size_t nameLength = strlen(nameBff);
		actualLayer->name = new char[nameLength + 1];
		std::memcpy(actualLayer->name, nameBff, nameLength);
		actualLayer->name[nameLength] = '\0';
	}

	//char* layerclass; /*class*/
	const char* layerclassBff = XML::GetAttributeContent("layerclass", _tag->attributesCount, _tag->attributes);
	if (layerclassBff != nullptr)
	{
		size_t layerclassLength = strlen(layerclassBff);
		actualLayer->layerclass = new char[layerclassLength + 1];
		std::memcpy(actualLayer->layerclass, layerclassBff, layerclassLength);
		actualLayer->layerclass[layerclassLength] = '\0';
	}

	{
		//int x;
		const char* xBff = XML::GetAttributeContent("x", _tag->attributesCount, _tag->attributes);
		actualLayer->x = (xBff != nullptr) ? std::atoi(xBff) : 0;
		//int y;
		const char* yBff = XML::GetAttributeContent("y", _tag->attributesCount, _tag->attributes);
		actualLayer->y = (yBff != nullptr) ? std::atoi(yBff) : 0;
	}

	{
		//int width;
		const char* widthBff = XML::GetAttributeContent("width", _tag->attributesCount, _tag->attributes);
		actualLayer->width = (widthBff != nullptr) ? std::atoi(widthBff) : 0;
		//int height;
		const char* heightBff = XML::GetAttributeContent("height", _tag->attributesCount, _tag->attributes);
		actualLayer->height = (heightBff != nullptr) ? std::atoi(heightBff) : 0;
	}

	//float opacity; /*0 to 1*/
	const char* opacityBff = XML::GetAttributeContent("opacity", _tag->attributesCount, _tag->attributes);
	actualLayer->opacity = (opacityBff != nullptr) ? std::atof(opacityBff) : 1.0f;

	//bool visible;
	const char* visibleBff = XML::GetAttributeContent("visible", _tag->attributesCount, _tag->attributes);
	actualLayer->visible = (visibleBff != nullptr) ? std::atoi(visibleBff) : 0;

	//int tintcolor;
	const char* tintcolorBff = XML::GetAttributeContent("tintcolor", _tag->attributesCount, _tag->attributes);
	actualLayer->tintcolor = (tintcolorBff != nullptr) ? std::atoi(tintcolorBff) : 0;

	//int offsetx;
	const char* offsetxBff = XML::GetAttributeContent("offsetx", _tag->attributesCount, _tag->attributes);
	actualLayer->offsetx = (offsetxBff != nullptr) ? std::atoi(offsetxBff) : 0;
	//int offsety;
	const char* offsetyBff = XML::GetAttributeContent("offsety", _tag->attributesCount, _tag->attributes);
	actualLayer->offsety = (offsetyBff != nullptr) ? std::atoi(offsetyBff) : 0;

	//int parallaxx;
	const char* parallaxxBff = XML::GetAttributeContent("parallaxx", _tag->attributesCount, _tag->attributes);
	actualLayer->parallaxx = (parallaxxBff != nullptr) ? std::atoi(parallaxxBff) : 0;

	//int parallaxy;
	const char* parallaxyBff = XML::GetAttributeContent("parallaxy", _tag->attributesCount, _tag->attributes);
	actualLayer->parallaxy = (parallaxyBff != nullptr) ? std::atoi(parallaxyBff) : 0;

	layersCount++;
}

void Tiled::Map::SetAttributes(const char* _buffer, XML::Tag*& mapTag)
{
	const char* versionBff = XML::GetAttributeContent("version", mapTag->attributesCount, mapTag->attributes);
	version = (versionBff != nullptr) ? std::atof(versionBff) : .0f;

	const char* tiledversionBff = XML::GetAttributeContent("tiledversion", mapTag->attributesCount, mapTag->attributes);
	tiledversion = (tiledversionBff != nullptr) ? std::atof(tiledversionBff) : .0f;

	const char* orientationBff = XML::GetAttributeContent("orientation", mapTag->attributesCount, mapTag->attributes);
	if (orientationBff != nullptr)
	{
		if (strcmp(orientationBff, "orthogonal"))
		{
			orientation = Map::Orthogonal;
		}
		else if (strcmp(orientationBff, "isometric"))
		{
			orientation = Map::Isometric;
		}
		else if (strcmp(orientationBff, "staggered"))
		{
			orientation = Map::Staggered;
		}
		else if (strcmp(orientationBff, "hexagonal"))
		{
			orientation = Map::Hexagonal;
		}
	}
	else
	{
		orientation = Map::Orthogonal;
	}

	const char* renderorderBff = XML::GetAttributeContent("renderorder", mapTag->attributesCount, mapTag->attributes);
	if (renderorderBff != nullptr)
	{
		if (strcmp(renderorderBff, "right-down"))
		{
			renderorder = Map::Right_Down;
		}
		else if (strcmp(renderorderBff, "right-up"))
		{
			renderorder = Map::Right_Up;
		}
		else if (strcmp(renderorderBff, "left-down"))
		{
			renderorder = Map::Left_Down;
		}
		else if (strcmp(renderorderBff, "left-up"))
		{
			renderorder = Map::Left_up;
		}
	}
	else
	{
		renderorder = Map::Right_Down;
	}

	const char* compressionlevelBff = XML::GetAttributeContent("compressionlevel", mapTag->attributesCount, mapTag->attributes);
	compressionlevel = (compressionlevelBff != nullptr) ? std::atoi(compressionlevelBff) : -1;

	{
		const char* widthBff = XML::GetAttributeContent("width", mapTag->attributesCount, mapTag->attributes);
		width = (widthBff != nullptr) ? std::atoi(widthBff) : 0;

		const char* heightBff = XML::GetAttributeContent("height", mapTag->attributesCount, mapTag->attributes);
		height = (heightBff != nullptr) ? std::atoi(heightBff) : 0;
	}

	{
		const char* tilewidthBff = XML::GetAttributeContent("tilewidth", mapTag->attributesCount, mapTag->attributes);
		tilewidth = (tilewidthBff != nullptr) ? std::atoi(tilewidthBff) : 0;

		const char* tileheightBff = XML::GetAttributeContent("tileheight", mapTag->attributesCount, mapTag->attributes);
		tileheight = (tileheightBff != nullptr) ? std::atoi(tileheightBff) : 0;
	}

	{
		const char* hexsidelengthBff = XML::GetAttributeContent("hexsidelength", mapTag->attributesCount, mapTag->attributes);
		hexsidelength = (hexsidelengthBff != nullptr) ? std::atoi(hexsidelengthBff) : 0;

		const char* staggeraxisBff = XML::GetAttributeContent("staggeraxis", mapTag->attributesCount, mapTag->attributes);
		staggeraxis = (staggeraxisBff != nullptr) ? std::atoi(staggeraxisBff) : 0;

		const char* staggerindexBff = XML::GetAttributeContent("staggerindex", mapTag->attributesCount, mapTag->attributes);
		staggerindex = (staggerindexBff != nullptr) ? std::atoi(staggerindexBff) : 0;

		const char* parallaxoriginxBff = XML::GetAttributeContent("parallaxoriginx", mapTag->attributesCount, mapTag->attributes);
		parallaxoriginx = (parallaxoriginxBff != nullptr) ? std::atoi(parallaxoriginxBff) : 0;

		const char* parallaxoriginyBff = XML::GetAttributeContent("parallaxoriginy", mapTag->attributesCount, mapTag->attributes);
		parallaxoriginy = (parallaxoriginyBff != nullptr) ? std::atoi(parallaxoriginyBff) : 0;
	}

	const char* backgroundColorBff = XML::GetAttributeContent("backgroundColor", mapTag->attributesCount, mapTag->attributes);
	backgroundcolor = (backgroundColorBff != nullptr) ? std::atoi(backgroundColorBff) : 0;

	const char* nextlayeridBff = XML::GetAttributeContent("nextlayerid", mapTag->attributesCount, mapTag->attributes);
	nextlayerid = (nextlayeridBff != nullptr) ? std::atoi(nextlayeridBff) : 0;

	const char* nextobjectidBff = XML::GetAttributeContent("nextobjectid", mapTag->attributesCount, mapTag->attributes);
	nextobjectid = (nextobjectidBff != nullptr) ? std::atoi(nextobjectidBff) : 0;

	const char* infiniteBff = XML::GetAttributeContent("infinite", mapTag->attributesCount, mapTag->attributes);
	infinite = (infiniteBff != nullptr) ? std::atoi(infiniteBff) : 0;
}


void Tiled::Reader::Tmx(const char* _fileName, Map& _map)
{
	//Tiled::Map map{};
	size_t directoryLength = strlen(MAP_DIRECTORY);
	size_t fileNameLength = strlen(_fileName);
	char* path = new char[fileNameLength + directoryLength + 1];
	std::memcpy(path, MAP_DIRECTORY, directoryLength);
	std::memcpy(path + directoryLength, _fileName, fileNameLength);
	path[directoryLength + fileNameLength] = '\0';

	XML::File file = XML::ReadFile(path);
	delete[] path;

	XML::Tag* mapTag = XML::GetTag("map", file.tagCount, file.tags);
	_map.LoadAttributes(mapTag);
	UpdateTilesets(mapTag, _map);
	UpdateLayers(mapTag, _map);
}

void Tiled::Reader::UpdateTilesets(XML::Tag* _mapTag, Map& _map)
{
	for (unsigned int i = 0; i < _mapTag->tagCount; i++)
	{
		if (strcmp(_mapTag->tags[i].name, "tileset") == 0)
		{
			_map.AddTileset(&_mapTag->tags[i]);
		}
	}
}


void Tiled::Reader::UpdateLayers(XML::Tag* _mapTag, Map& _map)
{
	for (unsigned int i = 0; i < _mapTag->tagCount; i++)
	{
		if (strcmp(_mapTag->tags[i].name, "layer") == 0)
		{
			_map.AddLayer(&_mapTag->tags[i]);
		}
	}
}

void Tiled::Reader::AddTileToGrid(tile& _tile, data& _data, unsigned int& _tileCount)
{
	if (_data.tiles != nullptr)
	{
		tile* tempGrid = new tile[_tileCount + 1];
		std::memcpy(tempGrid, _data.tiles, _tileCount * sizeof(tile));
		delete[] _data.tiles;
		_data.tiles = tempGrid;
	}
	else
	{
		_data.tiles = new tile[1];
	}

	if (_tile.originalId != 0)
	{
		_tile.horizontalFlip = _tile.originalId & Flags::FLIPPED_HORIZONTALLY;
		_tile.verticalFlip = _tile.originalId & Flags::FLIPPED_VERTICALLY;
		_tile.diagonalFlip = _tile.originalId & Flags::FLIPPED_DIAGONALLY;
		_tile.hexagonal120Rotation = _tile.originalId & Flags::ROTATED_HEXAGONAL_120;

		std::int64_t global_tile_id = _tile.originalId;

		global_tile_id &= ~Flags::FLIPPED_HORIZONTALLY;
		global_tile_id &= ~Flags::FLIPPED_VERTICALLY;
		global_tile_id &= ~Flags::FLIPPED_DIAGONALLY;
		global_tile_id &= ~Flags::ROTATED_HEXAGONAL_120;

		_tile.id = global_tile_id;
	}
	else
	{
		_tile.id = 0;
	}

	_data.tiles[_tileCount] = _tile;

	_tileCount++;
}

void Tiled::Reader::ReadCsvGrid(data& _data, const char* _buffer, unsigned int& _tileCount)
{
	unsigned int charCounter = 0;
	unsigned int line = 0;
	unsigned int collumn = 0;
	if (_data.tiles != nullptr)
	{
		delete[] _data.tiles;
		_data.tiles = nullptr;
	}

	char* searchBuffer = (char*)_buffer;

	size_t bufferLength = strlen(_buffer);
	const void* newlineAddr = std::memchr(searchBuffer, '\n', strlen(searchBuffer));
	int newlineIndex = bufferLength;
	if (newlineAddr != nullptr)
	{
		char* newlinePos = (char*)newlineAddr;
		newlineIndex = (int)(newlinePos - searchBuffer);
	}

	int actualLine = 0;
	bool searching = true;
	while (searching)
	{
		tile actualTile;
		const void* entryAddr = std::memchr(searchBuffer, ',', strlen(searchBuffer));
		if (entryAddr != nullptr)
		{
			char* entryPos = (char*)entryAddr;
			int entryIndex = (int)(entryPos - searchBuffer);

			newlineAddr = std::memchr(searchBuffer, '\n', strlen(searchBuffer));
			if (newlineAddr != nullptr)
			{
				char* newlinePos = (char*)newlineAddr;
				newlineIndex = (int)(newlinePos - searchBuffer);
			}

			if (entryIndex < newlineIndex)
			{
				char* content = new char[entryIndex + 1];
				std::memcpy(content, searchBuffer, entryIndex);
				content[entryIndex] = '\0';

				actualTile.originalId = _atoi64(content);

				delete[] content;
				searchBuffer += entryIndex + 1;
			}
			else
			{
				char* content = new char[newlineIndex + 1];
				std::memcpy(content, searchBuffer, newlineIndex);
				content[entryIndex] = '\0';

				actualTile.originalId = _atoi64(content);

				delete[] content;
				actualLine++;
				searchBuffer += newlineIndex + 1;
			}
		}
		else
		{
			size_t contentLength = strlen(searchBuffer);
			char* content = new char[contentLength + 1];
			std::memcpy(content, searchBuffer, contentLength);
			content[contentLength] = '\0';

			actualTile.originalId = _atoi64(content);

			delete[] content;
			searching = false;
		}
		AddTileToGrid(actualTile, _data, _tileCount);
	}
}
