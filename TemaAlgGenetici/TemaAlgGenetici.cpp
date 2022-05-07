#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <cmath>

using namespace std;

ofstream out("date.out");

class Genetic {
	int n;	// Dimensiunea populatiei
	pair <int, int> D;	// Domeniul de definitie al functiei
	struct Parametrii {
		int a;
		int b;
		int c;
	}param;	// Parametrii pentru functia de maximizat
	int p;	// Precizia
	float pc;	// Probabilitatea de recombinare
	float pm;	// Probabilitatea de mutatie
	int nr;	// Numarul de etape ale algoritmului

	vector <vector <int>> populatie; // Un vector de cromozomi
	vector <float> valori_crom; // Valorile fiecarui cromozom
	vector <int> elitist;	// Cromozomul elitistului

public:
	// Utility functions
	void set_random_seed() {
		srand(time(0));
	}

	int calculeaza_lungime_cromozom() {
		return ceil(log2((D.second - D.first) * pow(10.0, p)));
	}

	float decodifica(vector <int>& cromozom, int a, int b) {
		int pow2 = 1;
		float val_cromozom = 0;
		for (int i = cromozom.size() - 1; i >= 0; i--) {
			val_cromozom += cromozom[i] * pow2;
			pow2 *= 2;
		}
		return (float)(b - a) / (float)(pow2 - 1) * val_cromozom + a;
	}

	float f(float x) {
		return param.a * x * x + param.b * x + param.c;
	}

	float calculeaza_prob_totala() {
		float total = 0;
		for (float elem : valori_crom) {
			total += f(elem);
		}
		return total;
	}

	float genereaza_u() {
		int l = calculeaza_lungime_cromozom();
		vector <int> aux;
		for (int i = 0; i < l; i++) {
			int r = rand() % 2;
			aux.push_back(r);
		}
		float u = decodifica(aux, 0, 1);
		return u;
	}

	bool conditie_terminare(int& t) {
		return t == nr;
	}

	int cautare_binara(float elem, vector <float>& intervale_prob) {
		int s = 0;
		int d = n;
		while (s <= d) {
			int m = (s + d) / 2;
			if (intervale_prob[m] == elem) {
				return m;
			}
			else if (intervale_prob[m] < elem) {
				s = m + 1;
			}
			else {
				d = m - 1;
			}
		}
		return s;
	}

	int genereaza_punct_taietura() {
		int l = calculeaza_lungime_cromozom();
		return rand() % l;
	}


	// Methods
	void load_data(string nume_fisier) {
		ifstream in(nume_fisier);

		in >> n;
		in >> D.first >> D.second;
		in >> param.a >> param.b >> param.c;
		in >> p;
		in >> pc;
		in >> pm;
		in >> nr;

		in.close();
	}

	void create_rand_population() {
		populatie.resize(n);
		int l_crom = calculeaza_lungime_cromozom();
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < l_crom; j++) {
				int random_num = rand() % 2;
				populatie[i].push_back(random_num);
			}
			valori_crom.push_back(decodifica(populatie[i], D.first, D.second));
		}
	}

	void scrie_populatia_initiala() {
		out << "Populatia initiala\n";
		for (int i = 0; i < n; i++) {
			out << i + 1 << ": ";
			for (int elem : populatie[i]) {
				out << elem;
			}
			float x = valori_crom[i];
			out << " x= " << x;
			out << " f= " << f(x);
			out << "\n";
		}
	}

	void selectie(bool afiseaza) {
		float prob_totala = calculeaza_prob_totala();
		vector <float> intervale_prob;
		intervale_prob.resize(n + 1);
		intervale_prob[0] = 0;
		if (afiseaza == true) {
			out << "\nProbabilitati selectie:\n";
		}
		for (int i = 1; i <= n; i++) {
			float prob = f(valori_crom[i - 1]) / prob_totala;	// Calculez probabilitatea de selectie a unui cromozom
			intervale_prob[i] = intervale_prob[i - 1] + prob;	// Determin probabilitatea cumulata pana la pasul i
			if (afiseaza == true) {
				out << "cromozom " << i << " probabilitate " << prob << "\n";
			}
		}
		if (afiseaza == true) {
			out << "\nIntervale probabilitati selectie\n";
			for (float prob : intervale_prob) {
				out << prob << " ";
			}
			out << "\n";
		}

		vector <vector <int>> populatie_noua;
		vector <float> valori_noi;
		populatie_noua.resize(n);

		float f_max = f(valori_crom[0]);	// Determin cazul elitist
		int poz_max = 0;
		for (int i = 1; i < n; i++) {
			if (f(valori_crom[i]) > f_max) {
				f_max = f(valori_crom[i]);
				poz_max = i;
			}
		}
		elitist = populatie[poz_max];

		for (int i = 0; i < n; i++) {	// Realizez selectia cromozomilor
			float u = genereaza_u();
			int poz_cromozom = cautare_binara(u, intervale_prob);
			if (afiseaza == true) {
				out << "u= " << u << " selectam cromozomul " << poz_cromozom << "\n";
			}
			populatie_noua[i] = populatie[poz_cromozom - 1];
			valori_noi.push_back(valori_crom[poz_cromozom -1]);
		}
		populatie = populatie_noua;
		valori_crom = valori_noi;
		if (afiseaza == true) {
			out << "Dupa selectie:\n";
			for (int i = 0; i < n; i++) {
				out << i + 1 << ": ";
				for (int elem : populatie[i]) {
					out << elem;
				}
				out << " x= " << valori_crom[i];
				out << " f= " << f(valori_crom[i]);
				out << "\n";
			}
		}
	}

	void incrucisare(bool afisare) {
		if (afisare == true) {
			out << "\nProbabilitatea de incrucisare " << pc << "\n";
		}
		vector <int> poz_marcate;	// Tinem minte pozitia cromozomilor care vor participa la incrucisare
		for (int i = 0; i < n; i++) {
			float u = genereaza_u();
			if (u < pc) {
				poz_marcate.push_back(i);
			}
			if (afisare == true) {
				out << i + 1 << ": ";
				for (int elem : populatie[i]) {
					out << elem;
				}
				out << " u= " << u;
				if (u < pc) {
					out << " < " << pc << " participa\n";
				}
				else {
					out << "\n";
				}
			}
		}

		for (int i = 1; i < poz_marcate.size(); i += 2) {	// Luam cromozomii 2 cate 2 si ii incrucisam
			int punct_taietura = genereaza_punct_taietura();
			vector <int> cromozom_nou_1;
			vector <int> cromozom_nou_2;
			for (int j = 0; j < populatie[i].size(); j++) {
				if (j < punct_taietura) {
					cromozom_nou_1.push_back(populatie[poz_marcate[i]][j]);
					cromozom_nou_2.push_back(populatie[poz_marcate[i - 1]][j]);
				}
				else {
					cromozom_nou_1.push_back(populatie[poz_marcate[i - 1]][j]);
					cromozom_nou_2.push_back(populatie[poz_marcate[i]][j]);
				}
			}

			if (afisare == true) {
				out << "Recombinare intre cromozomul " << poz_marcate[i - 1] + 1 << " si " << poz_marcate[i] + 1 << ":\n";
				for (int elem : populatie[poz_marcate[i - 1]]) {
					out << elem;
				}
				out << " ";
				for (int elem : populatie[poz_marcate[i]]) {
					out << elem;
				}
				out << " punct " << punct_taietura << "\n";
				out << "Rezultat ";
				for (int elem : cromozom_nou_1) {
					out << elem;
				}
				out << " ";
				for (int elem : cromozom_nou_2) {
					out << elem;
				}
				out << "\n";
			}

			populatie[poz_marcate[i - 1]] = cromozom_nou_1;
			valori_crom[poz_marcate[i - 1]] = decodifica(populatie[poz_marcate[i - 1]], D.first, D.second);
			populatie[poz_marcate[i]] = cromozom_nou_2;
			valori_crom[poz_marcate[i]] = decodifica(populatie[poz_marcate[i]], D.first, D.second);
		}

		if (afisare == true) {
			out << "Dupa recombinare:\n";
			for (int i = 0; i < n; i++) {
				out << i + 1 << ": ";
				for (int elem : populatie[i]) {
					out << elem;
				}
				out << " x= " << valori_crom[i];
				out << " f= " << f(valori_crom[i]);
				out << "\n";
			}
		}
	}

	void mutatie(bool afisare) {
		if (afisare == true) {
			out << "\nProbabilitatea de mutatie pentru fiecare gena " << pm << "\n";
			out << "Au fost modificati cromozomii: \n";
		}
		for (int i = 0; i < n; i++) {
			float u = genereaza_u();
			if (u < pm) {
				int poz = genereaza_punct_taietura();
				if (populatie[i][poz] == 1)
					populatie[i][poz] = 0;
				else
					populatie[i][poz] = 1;
				valori_crom[i] = decodifica(populatie[i], D.first, D.second);
				if (afisare == true) {
					out << i + 1 << "\n";
				}
			}
		}

		if (afisare == true) {
			out << "Dupa mutatie:\n";
			for (int i = 0; i < n; i++) {
				out << i + 1 << ": ";
				for (int elem : populatie[i]) {
					out << elem;
				}
				out << " x= " << valori_crom[i];
				out << " f= " << f(valori_crom[i]);
				out << "\n";
			}
		}
	}

	void adauga_elitist() {
		n++;
		populatie.push_back(elitist);
		valori_crom.push_back(decodifica(elitist, D.first, D.second));
	}

	void afisare_max() {
		float f_max = f(valori_crom[0]);
		float sum_f = f(valori_crom[0]);
		for (int i = 1; i < n; i++) {
			if (f(valori_crom[i]) > f_max) {
				f_max = f(valori_crom[i]);
			}
			sum_f += f(valori_crom[i]);
		}
		out << "Valoarea maxima= " << f_max << "; Valoarea media a performantei= " << sum_f / n << "\n";
	}



	// Test methods
	void load_ex() {
		populatie.resize(n);

		//populatie[0] = { 1,0,1,0,0,1,0,1,1,0,0,0,1,1,0,1,0,1,0,1,0,0 };
		//valori_crom.push_back(decodifica(populatie[0], D.first, D.second));
		populatie[0] = { 0,0,0,0,0,1,1,1,0,1,0,0,1,0,0,1,1,1,0,0,0,1 };
		valori_crom.push_back(decodifica(populatie[0], D.first, D.second));
		populatie[1] = { 0,0,1,0,1,0,0,1,0,0,1,1,1,0,1,1,1,1,1,1,0,0 };
		valori_crom.push_back(decodifica(populatie[1], D.first, D.second));
		populatie[2] = { 0,1,0,0,0,0,0,0,0,1,0,1,0,0,1,0,1,1,0,1,1,1 };
		valori_crom.push_back(decodifica(populatie[2], D.first, D.second));
		populatie[3] = { 1,1,0,1,0,0,1,1,1,0,1,1,0,0,0,1,1,0,1,1,0,1 };
		valori_crom.push_back(decodifica(populatie[3], D.first, D.second));
		populatie[4] = { 1,0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,0,0,1 };
		valori_crom.push_back(decodifica(populatie[4], D.first, D.second));
		populatie[5] = { 1,0,1,1,1,1,1,0,0,1,0,0,0,0,1,1,0,0,0,1,0,0 };
		valori_crom.push_back(decodifica(populatie[5], D.first, D.second));
		populatie[6] = { 0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,1,1,1,1 };
		valori_crom.push_back(decodifica(populatie[6], D.first, D.second));
		populatie[7] = { 0,0,0,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,0 };
		valori_crom.push_back(decodifica(populatie[7], D.first, D.second));
		populatie[8] = { 0,1,0,1,1,1,0,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1 };
		valori_crom.push_back(decodifica(populatie[8], D.first, D.second));
		populatie[9] = { 0,0,1,1,1,0,1,0,1,0,1,1,0,1,1,0,0,0,1,0,0,1 };
		valori_crom.push_back(decodifica(populatie[9], D.first, D.second));
		populatie[10] = { 1,1,0,0,1,1,0,1,1,1,0,1,0,0,1,0,0,1,1,1,1,1 };
		valori_crom.push_back(decodifica(populatie[10], D.first, D.second));
		populatie[11] = { 0,1,1,1,0,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,1 };
		valori_crom.push_back(decodifica(populatie[11], D.first, D.second));
		populatie[12] = { 1,1,1,1,1,1,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0 };
		valori_crom.push_back(decodifica(populatie[12], D.first, D.second));
		populatie[13] = { 0,1,1,1,0,1,0,0,0,0,0,0,0,0,1,0,1,1,0,0,1,0 };
		valori_crom.push_back(decodifica(populatie[13], D.first, D.second));
		populatie[14] = { 1,1,0,0,0,0,0,0,0,1,1,1,0,1,1,1,0,0,0,1,0,1 };
		valori_crom.push_back(decodifica(populatie[14], D.first, D.second));
		populatie[15] = { 1,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,1,0,1,1 };
		valori_crom.push_back(decodifica(populatie[15], D.first, D.second));
		populatie[16] = { 1,1,0,1,0,1,1,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1 };
		valori_crom.push_back(decodifica(populatie[16], D.first, D.second));
		populatie[17] = { 1,1,0,1,1,0,1,1,1,0,1,0,0,0,1,0,0,1,1,1,1,1 };
		valori_crom.push_back(decodifica(populatie[17], D.first, D.second));
		populatie[18] = { 0,0,1,0,0,1,0,1,0,1,0,1,1,0,0,1,0,1,1,1,1,0 };
		valori_crom.push_back(decodifica(populatie[18], D.first, D.second));
		populatie[19] = { 1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,1,0,0,1,1,1 };
		valori_crom.push_back(decodifica(populatie[19], D.first, D.second));
	}

	void print_data(bool afiseaza_populatie) {
		cout << "Dimensiunea populatiei = " << n << "\n";
		cout << "Domeniul de definitie al functiei = [" << D.first << ", " << D.second << "]\n";
		cout << "f(x) = (" << param.a << ")*X^2 + (" << param.b << ")*X +(" << param.c << ")\n";
		cout << "Precizia = " << p << "\n";
		cout << "Probabilitatea de recombinare = " << pc << "\n";
		cout << "Probabilitatea de mutatie = " << pm << "\n";
		cout << "Numarul de etape ale algoritmului = " << nr << "\n";
		if (afiseaza_populatie == true) {
			cout << "Populatie: \n";
			for (auto cromozom : populatie) {
				for (auto elem : cromozom) {
					cout << elem;
				}
				cout << "\n";
			}
		}
	}
};

void run() {
	Genetic alg;

	alg.load_data("date.in");
	int t = 0;
	alg.set_random_seed();
	alg.create_rand_population();
	//alg.load_ex();
	alg.scrie_populatia_initiala();
	while (alg.conditie_terminare(t) == false) {
		alg.selectie(t == 0);
		alg.incrucisare(t == 0);
		alg.mutatie(t == 0);
		alg.adauga_elitist();
		alg.afisare_max();
		t++;
	}

	out.close();
}

int main() {
	run();
	return 0;
}