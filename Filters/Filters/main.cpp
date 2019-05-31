#include <cstdlib>
#include <glut.h>
#include <glaux.h>
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream>

#pragma comment(lib, "legacy_stdio_definitions.lib")

using namespace std;
unsigned char *ifMArray;//////////////////////////////////////
unsigned short int 
x, //Ширина изображения
y; //Высота изображения
string FileName; //Имя фала
unsigned int binar; //Порог бинаризации
unsigned char
d,//Глубина цвета (бит/пиксель)
m,//Монохросность
countfilters,//Количество фильтров
aperture,//Размер апертуры
a_or_m;//Усредняющий или медианный (флаг)
int Hist[256] = {0}, HistCopy[256];
int maxH = 0, minH = 0;//Максимальное и минимаьлное значение гистограммы

unsigned int Texture[1];

void HoaraSort(unsigned char* Array, int first, int last)
{ 
	int i = first, j = last; 
	double tmp, x = Array[(first + last) / 2];
	do { 
			while (Array[i] < x)
			i++;    
				while (Array[j] > x)
					j--;      
					if (i <= j)
					{ 
						if (i < j)
						{ 
							tmp = Array[i];
							Array[i] = Array[j];
							Array[j] = tmp;
						}     
						i++; 
						j--;
					}
		} while (i <= j);
	if (i < last)
		HoaraSort(Array, i, last);
	if (first < j)
		HoaraSort(Array, first, j);
}

void Histogram(unsigned char *Array)
{
	int count = 0, H = 0;
	HoaraSort(Array, 0, x*y - 1);

	H = Array[0];
	for (int i = 0; i < x*y; i++)
	{
		if (H != Array[i])
		{
			Hist[H] = count;
			H = Array[i];
			count = 1;
		}
		else
		{
			count++;				
		}
		if (i == (x*y - 1))
		{
			Hist[H] = count;
		}
	}
	maxH = Hist[0];
	minH = Hist[0];
	for (int i = 1; i < 256; i++)
	{
		if (maxH < Hist[i])
		{
			maxH = Hist[i];
		}
		if (minH > Hist[i])
		{
			minH = Hist[i];
		}
	}
	for (int i = 0; i < 256; i++)
	{
		HistCopy[i] = Hist[i];
	}
}

void Binarizator(unsigned char **Array, int wr)
{
	unsigned char **ifArray = new unsigned char *[y];
	for (int i = 0; i < y; i++)
	{
		ifArray[i] = new unsigned char [x];
	}
	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			ifArray[i][j] = Array[i][j];
		}
	}
	int k = 0;
	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			if (ifArray[i][j] >= binar)
			{
				ifArray[i][j] = 255;
			}
			else
			{
				ifArray[i][j] = 0;
			}
		}
	}
	unsigned char *BinarArr = new unsigned char[x*y];
	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			BinarArr[k] = ifArray[i][j];
			k++;
		}
	}
	Histogram(BinarArr);

	//Запись в файл
	if (wr == 1)
	{
		string FileNameBin = FileName;
		FileNameBin.size() - 3;
		FileNameBin.replace(FileNameBin.size() - 3, FileNameBin.size(), "bin", 3);

		ofstream offile(FileNameBin.c_str(), ios::binary | ios::out);
		offile.write((char*)&x, 2);	//Ширина изображения
		offile.write((char*)&y, 2);	//Высота изображения
		offile.write((char*)&d, 1);	//Глубина цвета (бит/пиксель)
		offile.write((char*)&m, 1);	//Монохросность

		//Изображение обработанное Бинаризатором

		for (int i = 0; i < y; i++)
		{
			offile.write((char*)ifArray[i], x);
		}
			
		offile.write((char*)&minH, 4);//Минимальное значение гистограммы
		offile.write((char*)&maxH, 4);//Максимальное значение гистограммы

		int countCell = pow(2, d);//Количество ячеек гистограммы
		offile.write((char*)&countCell, 4);

		//Гистограмма
		offile.write((char*)&Hist, sizeof(Hist));
		
		offile.close();

		printf("Файл .bin создан\n");
	}
	for (int i = 0; i < 256; i++)
	{
		Hist[i] = 0;
	}
	for (int i = 0; i < y; i++)
		delete[]ifArray[i];
}

void AveragingFilter(unsigned char **Mask, unsigned char **Array, int wr, int cf)
{
	int sum = 0, count = 0, k = 0, w = 0, h = 0, HalfAperture = floor(aperture / 2);
	unsigned char **ifArray = new unsigned char *[y];
	for (int i = 0; i < y; i++)
	{
		ifArray[i] = new unsigned char[x];
	}
	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			ifArray[i][j] = Array[i][j];
		}
	}
	//Picture
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			//Aperture
			for (int m = 0; m < aperture; m++)
			{
				for (int n = 0; n < aperture; n++)
				{
					w = (i - HalfAperture) + n;
					h = (j - HalfAperture) + m;
					if (!((w < 0) || (h < 0) || (w >= x) || (h >= y)))
					{
						sum += (ifArray[h][w] * Mask[m][n]);
						count += Mask[m][n];
					}
				}
			}
			if (count == 0)
			{
				count = 1;
			}
			ifArray[j][i] = round(sum / count);
			count = 0;
			sum = 0;
		}
	}
	unsigned char *AveragingArray = new unsigned char[x*y];
	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			AveragingArray[k] = ifArray[i][j];
			k++;
		}
	}
	
	Hist[256] = { 0 };
	Histogram(AveragingArray);
	
	//Запись в файл
	if (wr == 1)
	{
		stringstream ufcf;
		ufcf << cf;
		string ufi, uf = "uf";
		ufcf >> ufi;
		uf += ufi;
		string FileNameAver = FileName;
		FileNameAver.size() - 3;
		FileNameAver.replace(FileNameAver.size() - 3, FileNameAver.size(), uf.c_str(), 3);

		ofstream offile(FileNameAver.c_str(), ios::binary | ios::out);
		offile.write((char*)&x, 2);	//Ширина изображения
		offile.write((char*)&y, 2);	//Высота изображения
		offile.write((char*)&d, 1);	//Глубина цвета (бит/пиксель)
		offile.write((char*)&m, 1);	//Монохросность
		offile.write((char*)&aperture, 1);//Апертура

		//Маска
		for (int i = 0; i < aperture; i++)
		{
			for (int j = 0; j < aperture; j++)
			{
				offile.write((char*)&Mask[i][j], 1);
			}
		}

		//Изображение обработанное Усредняющим фильтром
		for (int i = 0; i < y; i++)
		{
			offile.write((char*)ifArray[i], x);
		}
		offile.write((char*)&minH, 4);//Минимальное значение гистограммы
		offile.write((char*)&maxH, 4);//Максимальное значение гистограммы

		int countCell = pow(2, d);//Количество ячеек гистограммы
		offile.write((char*)&countCell, 4);

		//Гистограмма
		offile.write((char*)&Hist, sizeof(Hist));

		offile.close();
		printf("Файл .uf создан\n");
	}
	for (int i = 0; i < 256; i++)
	{
		Hist[i] = 0;
	}
	for (int i = 0; i < y; i++)
		delete[]ifArray[i];
}

void MedianFilter(unsigned char **Array, int wr)
{
	int sum = 0, count = 0, k = 0, w = 0, h = 0, HalfAperture = floor(aperture / 2);
	unsigned char **ifArray = new unsigned char *[y];
	for (int i = 0; i < y; i++)
	{
		ifArray[i] = new unsigned char[x];
	}
	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			ifArray[i][j] = Array[i][j];
		}
	}
	unsigned char *Mask = new unsigned char[aperture*aperture];

	//Picture
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			//Aperture
			for (int m = 0; m < aperture; m++)
			{
				for (int n = 0; n < aperture; n++)
				{
					w = (i - HalfAperture) + n;
					h = (j - HalfAperture) + m;
					if (!((w < 0) || (h < 0) || (w >= x) || (h >= y)))
					{
						Mask[k] = ifArray[h][w];
						k++;
					}
				}
			}
			HoaraSort(Mask, 0, k - 1);
			ifArray[j][i] = Mask[k/2];
			count = 0;
			k = 0;
		}
	}

	delete[] Mask;
	k = 0;
	unsigned char *MedianArray = new unsigned char[x*y];
	for (int i = 0; i < y; i++)
	{
		for (int j = 0; j < x; j++)
		{
			MedianArray[k] = ifArray[i][j];
			k++;
		}
	}

	ifMArray = new unsigned char[x*y];
	for (int i = 0; i < x*y; i++)
	{
		ifMArray[i] = MedianArray[i];
	}

	Histogram(MedianArray);
	
	//Запись в файл
	if (wr == 1)
	{
		string FileNameMed = FileName;
		FileNameMed.size() - 3;
		FileNameMed.replace(FileNameMed.size() - 3, FileNameMed.size(), "med", 3);

		ofstream offile(FileNameMed.c_str(), ios::binary | ios::out);
		offile.write((char*)&x, 2);	//Ширина изображения
		offile.write((char*)&y, 2);	//Высота изображения
		offile.write((char*)&d, 1);	//Глубина цвета (бит/пиксель)
		offile.write((char*)&m, 1);	//Монохросность
		offile.write((char*)&aperture, 1);//Апертура

		//Изображение обработанное Медианным фильтром
		for (int i = 0; i < y; i++)
		{
			offile.write((char*)ifArray[i], x);
		}
		offile.write((char*)&minH, 4);//Минимальное значение гистограммы
		offile.write((char*)&maxH, 4);//Максимальное значение гистограммы

		int countCell = pow(2, d);//Количество ячеек гистограммы
		offile.write((char*)&countCell, 4);

		//Гистограмма
		offile.write((char*)&Hist, sizeof(Hist));
		
		offile.close();
		printf("Файл .med создан\n");
	}
	for (int i = 0; i < 256; i++)
	{
		Hist[i] = 0;
	}
	for (int i = 0; i < y; i++)
		delete[]ifArray[i];
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Russian");
	cout << "Введите имя файла: ";
	getline(cin, FileName);
	
	ifstream iffile(FileName.c_str(), ios::binary | ios::in);
	if (iffile.is_open() == false)
	{
		printf("Unable to open file\n");
		exit(1);
	}
	iffile.read((char*)&x, 2);	//Ширина изображения
	iffile.read((char*)&y, 2);	//Высота изображения
	iffile.read((char*)&d, 1);	//Глубина цвета (бит/пиксель)
	iffile.read((char*)&m, 1);	//Монохросность

	//Изображение
	unsigned char **ArrayPic = new unsigned char*[y];
	for (int i = 0; i < y; i++)
	{
		ArrayPic[i] = new unsigned char[x];
	}

	for (int i = 0; i < y; i++)
	{
		iffile.read((char*)ArrayPic[i], x);
	}
	iffile.read((char*)&binar, 4); //Порог бинарицзации

	Binarizator(ArrayPic, 1);

	iffile.read((char*)&countfilters, 1);

	unsigned char **mask_a;
	int countAver = 1;

	printf("Размер картинки   ширина: %d   высота: %d\n", x, y);
	printf("Глубина цвета (бит/пиксель): %d\tМонохромность: %d\tПорог бинаризации: %d\tКоличество фильтров: %d\n\n\n", d, m, binar, countfilters);

	for (int i = 0; i < countfilters; i++)
	{
		iffile.read((char*)&aperture, 1);//Апертура
		iffile.read((char*)&a_or_m, 1);//Усредняющий/Медианный фильтр

		printf("Апертура: %d\tУсредняющий = 0/Медианный фильтр = 1:   %d\n", aperture, a_or_m);
		if (a_or_m == 0)
		{
			mask_a = new unsigned char *[aperture];
			for (int i = 0; i < aperture; i++)
			{
				mask_a[i] = new unsigned char[aperture];
			}
			for (int i = 0; i < aperture; i++)
			{
				for (int j = 0; j < aperture; j++)
				{
					iffile.read((char*)&mask_a[i][j], 1);
				}
			}
			for (int i = 0; i < aperture; i++)
			{
				for (int j = 0; j < aperture; j++)
				{
					printf("% d \t", mask_a[i][j]);
				}
				printf("\n");
			}
			printf("\n");
			AveragingFilter(mask_a, ArrayPic, 1, countAver);
			countAver++;
			for (int i = 0; i < aperture; i++)
				delete[]mask_a[i];
		}
		else if (a_or_m == 1)
		{	
			MedianFilter(ArrayPic, 1);
		}
	}
	iffile.close();

	system("Pause");
}