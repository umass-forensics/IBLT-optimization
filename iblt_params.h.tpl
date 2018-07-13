/*
Copyright (c) 2018 The Bitcoin Unlimited developers
Distributed under the MIT software license, see the accompanying
file COPYING or http://www.opensource.org/licenses/mit-license.php.

This file has been auto-generated using a template found in the 
following repository. The template was populated using data 
generated with a script that is also found in this repository.

XXX
*/
#include <map>

class IbltParamItem 
{
public:
	float overhead;	
	uint8_t numhashes;

	IbltParamItem(float _overhead, uint8_t _numhashes)
	{
		IbltParamItem::overhead = _overhead;
		IbltParamItem::numhashes = _numhashes;
	}

};

const IbltParamItem DEFAULT_PARAM_ITEM(1.5, 3);

class CIbltParams
{
public:
    static std::map<size_t, IbltParamItem> paramMap;
	static IbltParamItem Lookup(size_t nItems)
	{
		auto pos = CIbltParams::paramMap.find(nItems);
		
		if (pos == CIbltParams::paramMap.end())
			return DEFAULT_PARAM_ITEM;
		else 
			return pos->second;
	}	
};

std::map<size_t, IbltParamItem> CIbltParams::paramMap = {
<<<items>>>
};
