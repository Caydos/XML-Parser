#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "XML.h"


#pragma region Constructor/Destructors
XML::Attribute::Attribute()
{
	lock = false;
	name = nullptr;
	content = nullptr;
}

XML::Attribute::~Attribute()
{
	if (!lock && name != nullptr)
	{
		delete[] name;
		name = nullptr;
	}
	if (!lock && content != nullptr)
	{
		delete[] content;
		content = nullptr;
	}
}

XML::Tag::Tag()
{
	lock = false;
	name = nullptr;
	attributesCount = 0;
	attributes = nullptr;
	content = nullptr;
	tagCount = 0;
	tags = nullptr;
}

XML::Tag::~Tag()
{
	if (!lock && name != nullptr)
	{
		delete[] name;
	}
	if (!lock && attributes != nullptr)
	{
		for (int i = 0; i < attributesCount; i++)
		{
			attributes[i].lock = false;
		}
		delete[] attributes;
	}
	if (!lock && content != nullptr)
	{
		delete[] content;
	}
	if (!lock && tags != nullptr)
	{
		for (int i = 0; i < tagCount; i++)
		{
			tags[i].lock = false;
		}
		delete[] tags;
	}
}


XML::File::File()
{
	name = nullptr;
	text = nullptr;
	tagCount = 0;
	tags = nullptr;
}
XML::File::~File()
{
	if (name != nullptr)
	{
		delete[] name;
	}
	if (text != nullptr)
	{
		delete[] text;
	}
	if (tags != nullptr)
	{
		for (int i = 0; i < tagCount; i++)
		{
			tags[i].lock = false;
		}
		delete[] tags;
	}
}

#pragma endregion

void XML::AddTag(Tag _tag, unsigned int& _tagCount, Tag*& _tags)
{
	if (_tagCount)
	{
		Tag* tempTags = new Tag[_tagCount + 1];
		std::memcpy(tempTags, _tags, _tagCount * sizeof(Tag));
		delete[] _tags;
		_tags = tempTags;
	}
	else
	{
		_tags = new Tag[1];
	}
	_tags[_tagCount] = _tag;
	_tagCount++;
}

void XML::SearchForTags(const char* _buffer, unsigned int& _tagCount, Tag*& _tags)
{
	if (_buffer == nullptr)
	{
		return;
	}
	char* searchBuffer = (char*)_buffer;

	bool reading = true;
	while (reading)
	{
		//std::cout << "Buffer : " << searchBuffer << std::endl;
		const void* entryAddr = std::memchr(searchBuffer, '<', strlen(searchBuffer));
		if (entryAddr != nullptr)
		{
			char* entryPos = (char*)entryAddr;
			int entryIndex = (int)(entryPos - searchBuffer);

			if (searchBuffer[entryIndex + 1] != '/')
			{//<xml>
				Tag tag;
				tag.lock = true;
				int additionalChar = (searchBuffer[entryIndex + 1] == '?') ? 1 : 0;
				searchBuffer += (entryIndex + 1/* index resolution */) + additionalChar;

				// Wsp search
				const void* wspAddr = std::memchr(searchBuffer, ' ', strlen(searchBuffer));
				if (wspAddr != nullptr)
				{
					char* wspPos = (char*)wspAddr;
					int wspIndex = (int)(wspPos - searchBuffer);

					size_t tagLength = wspIndex;
					tag.name = new char[tagLength + 1];
					std::memcpy(tag.name, searchBuffer, tagLength);
					tag.name[tagLength] = '\0';
					searchBuffer += tagLength + 1;
				}
				//std::cout << "Search for tag : " << tag.name << std::endl;

				// End Search
				bool foundTagEnding = false;
				int firstEndIndex = 0; // GetFirst closing for attributes
				char* attributeStart = searchBuffer;
				while (!foundTagEnding)
				{
					const void* endAddr = std::memchr(searchBuffer, '>', strlen(searchBuffer));
					if (endAddr != nullptr)
					{
						char* endPos = (char*)endAddr;
						int endIndex = (int)(endPos - searchBuffer);
						if (!firstEndIndex)
						{
							// Search for attributes
							int endAdditionalChar = (searchBuffer[endIndex - 1] == '?' || searchBuffer[endIndex - 1] == '/') ? 1 : 0;

							int finalIndex = endIndex - endAdditionalChar;
							char* attributeBuffer = new char[finalIndex + 1];
							std::memcpy(attributeBuffer, searchBuffer, finalIndex);
							attributeBuffer[finalIndex] = '\0';

							GetAttributesFromBuffer(attributeBuffer, tag);
							delete[] attributeBuffer;

							firstEndIndex = endIndex;
							if (searchBuffer[endIndex - 1] == '/' || searchBuffer[endIndex - 1] == '?')
							{// <_name XXXX />
								//std::cout << "Found / ending" << std::endl;
								foundTagEnding = true;
							}
						}
						else
						{// <_name XXXX > </_name>
							size_t tagLength = strlen(tag.name);
							char* nameAv = new char[tagLength + 1];
							std::memcpy(nameAv, searchBuffer + endIndex - tagLength, tagLength);
							nameAv[tagLength] = '\0';

							if (searchBuffer[endIndex - tagLength - 1] == '/' && strcmp(nameAv, tag.name) == 0)
							{
								char* start = attributeStart + firstEndIndex + 1;

								size_t contentLength = (strlen(start) - strlen(searchBuffer)) + (/*</_name>*/endIndex - tagLength - 2 /* </ */);
								tag.content = new char[contentLength + 1];
								std::memcpy(tag.content, start, contentLength);
								tag.content[contentLength] = '\0';
								//std::cout << "Content : " << tag.content << std::endl;
								SearchForTags(tag.content, tag.tagCount, tag.tags);
								foundTagEnding = true;
							}
							delete[] nameAv;
						}
						searchBuffer += endIndex + 1;
					}
					else
					{
						std::cout << "Warning : No end found for tag " << tag.name << std::endl;
						foundTagEnding = true;
					}
				}

				AddTag(tag, _tagCount, _tags);
			}
			else
			{//</name>
				searchBuffer += entryIndex + 1;
				const void* endAddr = std::memchr(searchBuffer, '>', strlen(searchBuffer));
				if (endAddr != nullptr)
				{
					char* endPos = (char*)endAddr;
					int endIndex = (int)(endPos - searchBuffer);
					searchBuffer += endIndex + 1;
				}
			}
		}
		else
		{
			reading = false;
		}
	}
}

void XML::GetAttributesFromBuffer(const char* _buffer, Tag& _tag)
{
	char* searchBuffer = (char*)_buffer;
	bool searching = true;
	while (searching)
	{
		size_t searcBufferLength = strlen(searchBuffer);
		const void* markerAddr = std::memchr(searchBuffer, '=', searcBufferLength);
		if (markerAddr != nullptr)
		{
			char* markerPos = (char*)markerAddr;
			int markerIndex = (int)(markerPos - searchBuffer);
			if (!_tag.attributesCount)
			{
				_tag.attributes = new Attribute[1];
			}
			else
			{
				Attribute* tempContainer = new Attribute[_tag.attributesCount + 1];
				std::memcpy(tempContainer, _tag.attributes, _tag.attributesCount * sizeof(Attribute));
				delete[] _tag.attributes;
				_tag.attributes = tempContainer;
			}

			_tag.attributes[_tag.attributesCount].lock = true;

			int wspIndex;
			for (wspIndex = markerIndex; wspIndex > 0; wspIndex--)
			{
				if (searchBuffer[wspIndex] == ' ')
				{
					wspIndex++;
					break;
				}
			}

			size_t nameLength = markerIndex - wspIndex;
			_tag.attributes[_tag.attributesCount].name = new char[nameLength + 1];
			std::memcpy(_tag.attributes[_tag.attributesCount].name, searchBuffer + wspIndex, nameLength * sizeof(char));
			_tag.attributes[_tag.attributesCount].name[nameLength] = '\0';

			int quoteIndex;
			for (quoteIndex = markerIndex + 2/* =" */; quoteIndex < searcBufferLength; quoteIndex++)
			{
				if (searchBuffer[quoteIndex] == '"')
				{
					quoteIndex--;
					break;
				}
			}
			size_t contentLength = (quoteIndex - 1) - markerIndex;
			_tag.attributes[_tag.attributesCount].content = new char[contentLength + 1];
			std::memcpy(_tag.attributes[_tag.attributesCount].content, searchBuffer + markerIndex + 2/* =" */, contentLength * sizeof(char));
			_tag.attributes[_tag.attributesCount].content[contentLength] = '\0';

			_tag.attributes[_tag.attributesCount].lock = true;

			//std::cout << _tag.attributes[_tag.attributesCount].name << "=" << _tag.attributes[_tag.attributesCount].content << std::endl;

			_tag.attributesCount++;
			searchBuffer += markerIndex + 1;
		}
		else
		{
			searching = false;
		}
	}
}

XML::File XML::ReadFile(const char* _fileName)
{
	File file;

	size_t nameLength = strlen(_fileName);
	file.name = new char[nameLength + 1];
	std::memcpy(file.name, _fileName, nameLength);
	file.name[nameLength] = '\0';
	file.text = GetFileAsString(_fileName);
	SearchForTags(file.text, file.tagCount, file.tags);

	return file;
}

char* XML::GetFileAsString(const char* _fileName)
{
	std::ifstream mapFile(_fileName);
	if (mapFile.fail())
	{
		std::cout << "Failed to load file : " << _fileName << std::endl;
		exit(1);
	}
	char* content = nullptr;
	while (mapFile.good())
	{
		std::string line;
		std::getline(mapFile, line);
		size_t lineLength = strlen(line.c_str());
		if (content != nullptr)
		{
			size_t oldSize = _mbstrlen(content);
			size_t lineSize = _mbstrlen(line.c_str());
			char* tempBuffer = new char[oldSize + lineSize + 1];
			std::memcpy(tempBuffer, content, oldSize);
			std::memcpy(tempBuffer + oldSize, line.c_str(), lineSize);
			tempBuffer[oldSize + lineSize] = '\0';
			delete[] content;

			content = tempBuffer;
		}
		else
		{
			content = new char[lineLength + 1];
			std::memcpy(content, line.c_str(), lineLength);
			content[lineLength] = '\0';
		}
	}
	mapFile.close();
	//std::cout << content << std::endl;
	return content;
}


XML::Tag* XML::GetTag(const char* _name, unsigned int _tagCount, Tag*& _tags)
{
	for (unsigned int i = 0; i < _tagCount; i++)
	{
		if (strcmp(_tags[i].name, _name) == 0)
		{
			return &_tags[i];
		}
	}
	return nullptr;
}

XML::Attribute* XML::GetAttribute(const char* _name, unsigned int _attributesCount, Attribute*& _attributes)
{
	for (unsigned int i = 0; i < _attributesCount; i++)
	{
		if (strcmp(_attributes[i].name, _name) == 0)
		{
			return &_attributes[i];
		}
	}
	return nullptr;
}

const char* XML::GetAttributeContent(const char* _name, unsigned int _attributesCount, Attribute*& _attributes)
{
	for (unsigned int i = 0; i < _attributesCount; i++)
	{
		if (strcmp(_attributes[i].name, _name) == 0)
		{
			return _attributes[i].content;
		}
	}
	return nullptr;
}
