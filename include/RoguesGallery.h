#pragma once

//All types are kinda in the NEL namespace

//#include <NEBUtil/FullName.h>
//#include <NEBUtil/StringHash.h>


//I want to make a log macro, the point of the log macro will be to take some inputs
// and log them in a manner that fits whatever environmet I'm using. It will need
// to be defined in whatever its using it. it will look something like this
#define LOG(mc_level, mc_context, ...) //logger::##mc_level##(mc_context, __VA_ARGS__)
//The general idea would be, that this is probably gonna be used for a Clib project,
// but I may have another logger so I might as well have some way to interpret that