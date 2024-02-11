#ifndef XML_H
#define XML_H


namespace XML
{

	class Attribute
	{
	public:
		bool lock;
		char* name;
		char* content;

		Attribute();
		~Attribute();
	};
	class Tag
	{
	public:
		bool lock;
		char* name;

		unsigned int attributesCount;
		Attribute* attributes;

		char* content;

		unsigned int tagCount;
		Tag* tags;

		Tag();
		~Tag();

	};

	class File
	{
	public:
		char* name;
		char* text;

		unsigned int tagCount;
		Tag* tags;

		File();
		~File();
	};

	void AddTag(Tag _tag, unsigned int& _tagCount, Tag*& _tags);
	void SearchForTags(const char* _buffer, unsigned int& _tagCount, Tag*& _tags);

	void GetAttributesFromBuffer(const char* _buffer, Tag& _tag);

	char* GetFileAsString(const char* _fileName);
	File ReadFile(const char* _fileName);

	Tag* GetTag(const char* _name, unsigned int _tagCount, Tag*& _tags);
	Attribute* GetAttribute(const char* _name, unsigned int _attributesCount, Attribute*& _attributes);
	const char* GetAttributeContent(const char* _name, unsigned int _attributesCount, Attribute*& _attributes);
}


#endif // !XML_H
