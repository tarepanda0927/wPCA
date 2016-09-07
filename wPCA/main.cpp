#include<iostream>
#include"model.h"
#include<fstream>
#include<vector>
#include "info.h"
#include "raw_io.h"


void get_list_txt(std::list< std::string > &list_txt, std::string path_list) {
	list_txt.clear();
	std::ifstream ifs(path_list, std::ios::in);
	//fail to open file
	if (!ifs) {
		std::cerr << "Cannot open file: " << path_list << std::endl;
		std::abort();
	}
	else {
		std::string buf;
		while (std::getline(ifs, buf)) {
			//skip empty strings
			if (buf.length()) {
				list_txt.push_back(buf);
			}
		}
	}
	ifs.close();
}



template<typename T>
void write_matrix_raw_and_txt(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& data, std::string filename)
{
	//////////////////////////////////////////////////////////////
	// Wの書き出し												//
	// rowが隠れ層の数，colが可視層の数							//			
	// 重みの可視化を行う場合は，各行を切り出してreshapeを行う  //
	//////////////////////////////////////////////////////////////
	size_t rows = data.rows();
	size_t cols = data.cols();
	Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Data;
	Data = data;
	std::ofstream fs1(filename + ".txt");
	fs1 << "rows = " << rows << std::endl;
	fs1 << "cols = " << cols << std::endl;
	fs1 << typeid(Data).name() << std::endl;
	fs1.close();
	std::vector<T> save_data(rows * cols);
	Data.resize(rows * cols, 1);
	for (size_t i = 0; i <save_data.size(); i++)
		save_data[i] = Data(i, 0);
	write_vector(save_data, filename + ".raw");
	Data.resize(rows, cols);
}

//long get_file_size(std::string filename)
//{
//	FILE *fp;
//	struct stat st;
//	if (fopen_s(&fp, filename.c_str(), "rb") != 0) {
//		std::cerr << "Cannot open file: " << filename << std::endl;
//		std::abort();
//	}
//	fstat(_fileno(fp), &st);
//	fclose(fp);
//	return st.st_size;
//}

template< class T >
void read_bin(T *p, size_t num, std::string filename) {
	FILE *fp;
	if (fopen_s(&fp, filename.c_str(), "rb") != 0) {
		std::cerr << "Cannot open file" << filename << std::endl;
		std::abort();
	}
	fread(p, sizeof(*p), num, fp);
	fclose(fp);
}

template< class T >
void write_bin(const T *p, size_t num, std::string file_name) {
	FILE *fp;
	fopen_s(&fp, file_name.c_str(), "wb");
	fwrite(p, sizeof(*p), num, fp);
	fclose(fp);
}

template< class T >
void write_text(const T *p, size_t num, std::string file_name) {
	FILE *fp;
	fopen_s(&fp, file_name.c_str(), "w");
	for (int i = 0; i<num; i++) {
		fprintf(fp, "%lf\n", p[i]);
	}
	fclose(fp);
}

int main(int argc, char *argv[]) {
	text_info input_info;
	input_info.input(argv[1]);
	//テキストデータ読み込み
	std::vector<std::string> fcase;
	std::vector<std::string> rcase;
	std::ifstream f_case(input_info.dir_list + input_info.case_flist);
	std::ifstream r_case(input_info.dir_list + input_info.case_rlist);
	std::string buf_ft;
	std::string buf_rt;
	while (f_case&& getline(f_case, buf_ft))
	{
		fcase.push_back(buf_ft);
	}
	while (r_case&& getline(r_case, buf_rt))
	{
		rcase.push_back(buf_rt);
	}
	for (int i = 0; i < fcase.size(); i++) {
		//CVの数だけフォルダを作る(test症例名)
		std::string dir_fcv = input_info.dir_out + "Fl/" + fcase[i] + "/";
		std::string dir_rcv = input_info.dir_out + "Ref/" + rcase[i] + "/";
		CheckTheFolder::checkExistenceOfFolder(input_info.dir_out + "Fl/" + fcase[i] + "/");
		CheckTheFolder::checkExistenceOfFolder(input_info.dir_out + "Ref/" + rcase[i] + "/");
		//入力テキスト作成
		std::ofstream Ref_list(dir_rcv + "input.txt");
		std::ofstream Fl_list(dir_fcv + "input.txt");
		std::ofstream Ref_sc(dir_rcv + "test.txt");
		std::ofstream Fl_sc(dir_fcv + "test.txt");
		for (int j = 0; j < fcase.size(); j++) {
			if (i == j)continue;
			Ref_list << input_info.dir_LS + "Ref/" + rcase[j] + ".raw" << std::endl;
			Fl_list << input_info.dir_LS + "Fl/" + fcase[j] + ".raw" << std::endl;
		}
		for (int j = 0; j < fcase.size(); j++) {
			Ref_sc << input_info.dir_LS + "Ref/" + rcase[j] + ".raw" << std::endl;
			Fl_sc << input_info.dir_LS + "Fl/" + fcase[j] + ".raw" << std::endl;
		}

		saito::model<double> wpcaF;
		saito::model<double> wpcaR;
		//係数変えてね
		wpcaF.WPCA(dir_fcv + "input.txt", input_info.a);
		wpcaR.WPCA(dir_rcv + "input.txt", input_info.a);
		//wpcaR.PCA(dir_fcv + "input.txt");
		
		wpcaF.output(dir_fcv);
		wpcaR.output(dir_rcv);

		/* get file list */
		std::list< std::string > file_listR;
		get_list_txt(file_listR, dir_rcv + "test.txt");
		std::list< std::string > file_listF;
		get_list_txt(file_listF, dir_fcv + "test.txt");

		/* load bin */
		const std::size_t num_of_dataF = file_listF.size();
		const std::size_t num_of_dataR = file_listR.size();

		std::cout << "(^^)" << std::endl;
		long num_of_dimXr = get_file_size(*file_listR.begin()) / sizeof(double);
		long num_of_dimXf = get_file_size(*file_listF.begin()) / sizeof(double);
		Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic > Xr(num_of_dimXr, num_of_dataR);
		Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic > Xf(num_of_dimXf, num_of_dataF);

		std::cout << "(^^)" << std::endl;
	    {
			FILE *fp;
			double *rx = &Xr(0, 0);
			for (auto s = file_listR.begin(); s != file_listR.end(); ++s) {
				fopen_s(&fp, (*s).c_str(), "rb");
				rx += fread(rx, sizeof(double), num_of_dimXr, fp);
				fclose(fp);
			}
			
		}

		std::cout << "(^^)" << std::endl;
		{
			FILE *fp;
			double *fx = &Xf(0, 0);
			for (auto s = file_listF.begin(); s != file_listF.end(); ++s) {
				fopen_s(&fp, (*s).c_str(), "rb");
				fx += fread(fx, sizeof(float), num_of_dimXf, fp);
				fclose(fp);
			}
		}

		std::cout << "(^^)" << std::endl;

		Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic > Yr;
		Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic > Yf;
		wpcaR.out_of_sample_W(Yr, Xr);
		wpcaF.out_of_sample_W(Yf, Xf);
		std::cout << "(^^)" << std::endl;
		std::string or_file = static_cast<std::string>(dir_rcv ) + "/mat";
		std::string of_file = static_cast<std::string>(dir_fcv ) + "/mat";
		write_matrix_raw_and_txt(Yr, or_file);
		write_matrix_raw_and_txt(Yf, of_file);
		std::ofstream matr(or_file + ".csv");
		std::ofstream matf(of_file + ".csv");
		for (int i = 0; i < Yr.rows(); i++) {
			for (int j = 0; j < Yr.cols(); j++) {
				matr << Yr(i, j) << ",";
			}
			matr << std::endl;
		}
		for (int i = 0; i < Yf.rows(); i++) {
			for (int j = 0; j < Yf.cols(); j++) {
				matf << Yf(i, j) << ",";
			}
			matf << std::endl;
		}

		
	}
	return EXIT_SUCCESS;
	system("pause");

}
