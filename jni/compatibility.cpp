// <copyright file="compatiblity.cpp" company="WordLogic">
// Copyright (c) 2001, 2013 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Reza Nezami</author>
// <email>rnezami@wordlogic.com</email>
// <date>2012-06-10</date>
// <summary>provides data structure which is shared between runtime code and offline tools which author dictionaries, as well as versioning structs.</summary>

#include "compatibility.h"

TableSpace gTableSpaces[NTABLESPACES] =
{
	{ 0,		NULL }	// reserved for serialized everb definitions, dynamically filled
,	{ 0,		NULL }	// reserved for serialized erule definitions, dynamically filled
};

