/*先頭で日本語を打ち込んでおけばソースツリーで表示したときに文字化けしないらしいので*/

#ifndef __INFO__
#define __INFO__

#include "nariinfocontroller.h"
#include "narifile.h"
#include "naricommon.h"
#include <string>

struct text_info
{
	std::string dir_LS;
	std::string dir_Ref;
	std::string dir_out;
	std::string dir_list;
	std::string case_flist;
	std::string case_rlist;
	double a;

	inline void input(const std::string &path) // テキストから入力情報を書き込み
	{
		nari::infocontroller info;
		info.load(path);

		dir_LS = nari::file::add_delim(info.get_as_str("dir_LSimg"));
		dir_out = nari::file::add_delim(info.get_as_str("dir_out"));
		dir_list = nari::file::add_delim(info.get_as_str("dir_txt"));
		case_flist = info.get_as_str("case_f");
		case_rlist = info.get_as_str("case_r");
		a = info.get_as_double("alpha");
		info.output();
	}
};


#endif