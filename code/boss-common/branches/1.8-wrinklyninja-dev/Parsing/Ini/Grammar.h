/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Header file for ini grammar definition.

#ifndef __BOSS_INI_GRAM_H__
#define __BOSS_INI_GRAM_H__

#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE 
#endif

#include "Parsing/Data.h"
#include "Parsing/Skipper.h"
#include "Common/Globals.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"

#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace boss {
	namespace unicode = boost::spirit::unicode;
	namespace phoenix = boost::phoenix;
	namespace qi = boost::spirit::qi;

	using namespace std;
	using namespace qi::labels;

	using qi::skip;
	using qi::eol;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;

	using unicode::char_;
	using unicode::no_case;
	using unicode::space;

	using boost::spirit::info;
	using boost::format;

	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	void SetVar(string var, string value) {
		cout << var << " = " << value << endl;
	}

	template <typename Iterator>
	struct ini_grammar : qi::grammar<Iterator, Skipper<Iterator>> {
		ini_grammar() : ini_grammar::base_type(ini, "ini grammar") {

			comment =
				lit("#") > *(char_ - eol);

			ini =
				*(eol | comment)
				> section % eol
				> *(eol | comment)
				> eoi;

			section =
				*eol
				>> (title
				> eol
				> (setting | comment) % +eol);

			title =
				lit("[") > +(char_ - eol);

			setting =
				(var
				> '='
				> value)[phoenix::bind(&SetVar, _1, _2)];

			var %=
				lexeme[+(char_ - '=')];

			value %=
				lexeme[+(char_ - eol)];

			//Give each rule names.
			comment.name("comment");
			ini.name("ini");
			section.name("section");
			title.name("title");
			setting.name("setting");
			var.name("variable");
			value.name("value");
		
			on_error<fail>(comment,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(ini,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(section,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(title,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(setting,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(var,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
			on_error<fail>(value,phoenix::bind(&ini_grammar<Iterator>::SyntaxError,this,_1,_2,_3,_4));
		}

		typedef Skipper<Iterator> skipper;

		qi::rule<Iterator, skipper> comment, ini, section, title, setting;
		qi::rule<Iterator, string(), skipper> var, value;
	
		void SyntaxError(Iterator const& /*first*/, Iterator const& last, Iterator const& errorpos, info const& what) {
			ostringstream out;
			out << what;
			string expect = out.str().substr(1,out.str().length()-2);
			if (expect == "eol")
				expect = "end of line";

			string context(errorpos, min(errorpos +50, last));
			boost::trim_left(context);

			LOG_ERROR("Ini Parsing Error: Expected a %s at \"%s\". Ini parsing aborted. No further settings will be applied.", expect.c_str(), context.c_str());
			
			expect = "&lt;" + expect + "&gt;";
			boost::replace_all(context, "\n", "<br />\n");
			string msg = (ParsingErrorFormat % expect % context).str();
			errorMessageBuffer.push_back(msg);
			return;
		}
	};
}


#endif