#include <limits>
#include <vector>
#include <stack>
#include <vector>
#include <set>
#include <cstdarg>
#include <cstdio>
#include <valarray>
#include <iostream>
#include <cstring>


class NullBuffer : public std::streambuf
{
public:
	int overflow(int c) { return c; }
};
#define MAX_BUFFER 102400

char buffer[MAX_BUFFER + 2];
using namespace std;

enum Preservations {CONST, VAR, PROCEDURE, BEGIN, END, DOT/*EOF*/, COMMA, SEMICOLON/*END OF STATEMENT*/, IF, THEN, ELSE, CALL, READ, WRITE,
	PLUS, MINUS, DIVISION, MULTIPLE, WHILE, DO, ASSIGNMENT, LP, RP, ODD, EQUAL, GREATER, LESS, GREATEREQ, LESSEQ, NOTEQ};
enum States { P, SP, C, V, PRO, CDEF, INIT, VDEF, ST, MORE, COND, EXP, EXP2, ARITH, ARI2, TERM, FACTS, MULDIV, FACT };
enum Operations{ INT, JMP, LIT, OPR, STO, CAL, JPC, LOD, SIO };//pl/0 USER MANUAL
const char *operations[] { "INT", "JMP", "LIT", "OPR", "STO", "CAL", "JPC", "LOD", "SIO" };
const char *preservations[] = { "CONST", "VAR", "PROCEDURE", "BEGIN", "END", ".", ",", ";", "IF", "THEN", "ELSE", "CALL","READ", "WRITE", "+", "-",
	"/", "*", "WHILE", "DO", ":=", "(", ")", "ODD", "=", ">", "<", ">=", "<=", "#"};
int *** states, **follows ;

inline int** init_st(int i, ...);
enum Type { PSR, SYM, NUM };
//short status_pers_match = {false}; FOR LATER
#define P(a)  a + 3
#define S(a) P(a) + NOTEQ + 1
#define DeS(a) a - 4 - NOTEQ 
#define _S(a) a >= S(0) && a <= S(FACT)
#define init_follow(n, ...) init_st((n + 1), __VA_ARGS__, -1)[0]
#define A_MOD P(EQUAL)

struct IDS {
	int type;
	int index;
	IDS(int type, int index) {
		this->type = type;
		this->index = index;
	}
}**pres;

struct HASH {

	IDS *id = 0;
	char* value;
	unsigned int* int_value = 0;
	HASH(char* value, int index)
	{
		this->value = value;
		int_value = NULL;
		if (value[0] < '0' || value[0] > '9')
			id = new IDS(1, index);
		else
		{ 
			id = new IDS(2, index);
			int_value = new unsigned int(0);
			int length = strlen(value);
			for (int i = 0; i < length; i++)
			{
				*int_value *= 10;
				*int_value += value[i] - 48;
			}
		}
	}
	~HASH()
	{
		if (int_value != NULL)
			delete(int_value);
		if (id)
			delete id;
	}
};

vector<HASH *> sym;
vector<HASH *> num;

struct GrammaUnit {
	int *follows;
};

vector<IDS*> identifiers;


void GETSYM() {
	char* bp = buffer, token;
	int n_syms = NOTEQ + 2;
	unsigned char flags[1024] = { true };
	int index = 0, lastLength = 0;
	int j = 0, n_pre = 0, n_ids = 0, n_ids_const, n_nums = 0, n_nums_const , i = 0, nums_offset = 0, n_pre_const = 0;
	bool takein = false, lastTaken = false;
	IDS *lastMatch;
	//init
	pres = new IDS*[NOTEQ + 1];
	for (j = 0; j <= NOTEQ; j++) {
		pres[j] = new IDS(0, j);
	}

	while (*bp) {
		takein = false;
		lastTaken = false;
		if (!index)
		{
			while (*bp == '\t' || *bp == '\n' || *bp == ' ' || *bp == '\r')
				bp++;
			n_pre = NOTEQ + 1;
			n_pre_const = n_pre + 1;
			n_nums_const = n_nums = num.size();
			n_ids_const = n_ids = sym.size();
			nums_offset = n_pre_const + n_ids_const + 1;
			n_syms = NOTEQ + n_nums_const + n_ids_const + 3;
			for (j = 0; j < n_syms; j++)
				flags[j] = false;

			if (*bp == '_' || (*bp >= 'a' && *bp <='z') || (*bp >= 'A' && *bp <= 'Z'))
				flags[nums_offset - 1] = true;
			else if (*bp >= '0' && *bp <= '9')
				flags[n_pre] = true;
			else
				flags[nums_offset - 1] = flags[n_pre] = true;
			lastMatch = 0;
			lastLength = 0;
		}
		//pre
		
		for (j = 0; j <= NOTEQ && n_pre > 0; j++)
			if (!flags[j])
			{
				token = preservations[j][index];
				if (!token)
				{
					flags[j] = 2;
					lastMatch = pres[j];
					lastLength = index - 1;
					lastTaken = true;
					n_pre--;
				}
				else {
					if (*bp != token && (*bp != (token+ 32) || token <'A' || token >'Z'))
					{
						flags[j] = true;
						n_pre--;
					}
					else {
						takein = true;
					}
				}
			}
		if (!flags[n_pre_const - 1])
		{
			//id-todo - 1
				//id-hasbeen
				for (i = 0; i < n_ids_const && n_ids > 0; i++)
				{
					if (!flags[n_pre_const + i])
					{

						if (sym[i]->value[index])
						{
							if (*bp == sym[i]->value[index])
									takein = true;
							else {
								flags[n_pre_const + i] = true;
								n_ids--;
							}
						}
						else if (!lastTaken)
						{
							flags[n_pre_const + i] = 2;
							lastMatch = sym[i]->id;
							lastLength = index - 1;
							n_ids--;
							lastTaken = true;
						}
					}
				}
			if (*bp == '_' || (*bp >= 'a' && *bp <= 'z') || (*bp >= 'A' && *bp <= 'Z') || (*bp >= '0' && *bp <= '9'))
			{
				takein = true;
			}
			else if (index != 0 && !lastTaken)
			{
				char* tmp = new char[index];
				for (j = 0; j < index; j++)
					tmp[j] = (bp-index)[j];
				tmp[index] = 0;
				sym.push_back(new HASH (tmp,n_ids_const));
				lastMatch = sym.back()->id;
				lastLength = index - 1;
				flags[n_pre_const - 1] = true;
			}
			else {
				flags[n_pre_const - 1] = true;
			}
		}
		else if (!flags[nums_offset - 1]) {
			
				for (i = 0; i < n_nums_const && n_nums > 0; i++)
					if (!flags[nums_offset + i])
					{
						if (num[i]->value[index])
						{
							if (*bp == num[i]->value[index])
								takein = true;
							else {
								flags[nums_offset + i] = true;
								n_nums --;
							}
						}
						else if (!lastTaken)
						{
							flags[nums_offset + i] = 2;
							lastMatch = num[i]->id;
							lastLength = index - 1;
							n_nums --;
							lastTaken = true;
						}
					}
			if ((*bp >= '0' && *bp <= '9'))
			{
				takein = true;
			}
			else if(index != 0 && !lastMatch){
				char* tmp = new char[index];
				for (j = 0; j < index; j++)
					tmp[j] = (bp - index)[j];
				tmp[index] = 0;
				
				num.push_back(new HASH(tmp, n_nums_const));
				lastMatch = num.back()->id;
				lastLength = index - 1;
				flags[n_pre + n_ids_const + 1] = true;
			}
			else
				flags[n_pre + n_ids_const + 1] = true;
		}
		
		if (takein)
		{
			bp++;
			index++;
		}
		else if (lastMatch)
		{
			identifiers.push_back(lastMatch);
			bp -= index - lastLength - 1;
			index = 0;
		}
		else
		{
			cout << "undefined identifier " << *bp << '\n';
			exit(1);
		}
	}

	if (index == 1 && *(bp - index) == '.')
		identifiers.push_back(pres[DOT]);
	for (IDS *var : identifiers)
		cout<<var->type<<'\t'<<var->index<< '\n';
	cout <<'\n' <<"Ints"<< '\n';
	for  (HASH *var : num)
		cout << *(var->int_value) << '\n';
	cout << '\n' << "Syms" << '\n';
	for  (HASH *var : sym)
		cout << var->value << '\n';
}


struct Instruction {
	int nop, lop, rop;
	Instruction(int nop, int lop, int rop) {
		this->nop = nop;
		this->lop = lop;
		this->rop = rop;
	}
} *firstjump;
vector<pair<int, unsigned int>> *propos[1024];
bool checklist[1024];
bool geting_proc = false;
vector<int> INTS;
int lv = -1, dx[1024] = { 0 };
void push();
template <typename T>
class cheque : public vector<T> {
public:
	void push_back(T&& _Val )
	{
		if (!checklist[lv])
		{
			if (lv)
				propos[lv - 1]->back().second = this->size();
			else
				firstjump->rop =  this->size();
			checklist[lv] = true;
			push();
		}
		printf("inserting instructions...");
		vector<T>::push_back(_Val);
	}
	void pb(T&& _Val)
	{
		vector<T>::push_back(_Val);
	}
};
cheque<Instruction *> instructions;
void push() {
	instructions.pb(new Instruction(INT, 0, INTS.back() + 1));
	INTS.pop_back();
}
inline int** init_st(int i, ...) {
	int j = 0, k = 0, t;
	int **p = new int*[16];
	va_list arg_ptr;
	va_start(arg_ptr, i);
	p[0] = new int[12];

	do {
		t = va_arg(arg_ptr, int);
		p[j][k++] = t;
		if (t == -2)
		{
			p[++j] = new int[8];
			k = 0;
		}
	} while (t != -1);
	p[j + 1] = new int(-3);

	va_end(arg_ptr);
	return p;
}

inline int is_not_mod(int modifier){
	return (modifier >= P(EQUAL) && modifier <= P(NOTEQ)) ? A_MOD : modifier;
}	

struct Reduced{
	int type;//S
	void* param;
	Reduced(int type, void* param){
		this->type = type;
		this->param = param;
	}
	Reduced(int type) { //P
		this->type = type;
	}
	~Reduced() {
		//free(param);
	}
};
class Indexes {
public:
	int i, j, k;
	Indexes(int i, int j, int k){
		this->i = i;
		this->j = j;
		this->k = k;
	}
	Indexes(){
		i = j = k = 0;
	}
	bool operator<(Indexes a) const
	{
		if (i == a.i)
			if (j == a.j)
				return k < a.k;
			else
				return j < a.j;
		else
			return i < a.i;
	}
};

struct Vartable {

};
stack<int> lastSP;
vector<Reduced *>  buf;
vector<pair<vector<pair<int, unsigned int>>*, int>> vartable;
vector<IDS *>::iterator it;
Reduced *nextSym = 0;
stack<set<Indexes>> archive;
struct Pos {
	int start, end;
};
Reduced* peekNext()
{
	if (!nextSym) {
		if (it != identifiers.end())
			switch ((*it)->type)
			{
			case 0:
				return new Reduced(P((*it)->index), NULL);
			case 1:
				return new Reduced(1, (void *)&(*it)->index);
			case 2:
				return new Reduced(2, (void *)num[(*it)->index]->int_value);
			default:
				printf("Unchecked list?");
				exit(1);
			}
		else
			return NULL;

	}
	else return nextSym;
}
int address = 0;
bool Next() {
	if (nextSym)
	{
		nextSym = NULL;
		return true;
	}
	else if (it == identifiers.end())
		return false;

	it++;
	address++;
	return true;
}
Reduced* getNext()
{
	Reduced *n = peekNext();
	Next();
	return n;
}

inline void pop(const int &i, const int &j) {
	int k = 0;
	while (states[i][j][k++]>0)
		buf.pop_back();
}
inline void popArchive(const int&i, const int &j) {
	int k = 0;
	while (states[i][j][k++] > 0)
		archive.pop();
}
inline bool searchTableforVar(const int& param, int &level, int &d, bool &found, const bool con = false)
{
	for  (pair<vector<pair<int, unsigned int>>*, int> var : vartable)
	{
		if (var.second == 1 || (var.second == 0 && con))
		{
			for  (pair<int, unsigned int> v : *(var.first))
			{
				if (v.first == param)
				{
					level /= 3;
					d = v.second;
					found = true;
					break;
				}
			}
			
			if (found)
				return (bool)var.second;
			
		}
		level++;
	}
	return false;
}

Reduced* reduce(int i, int j) {

	void *param = new int(0);
	switch (i)
	{
	case P:
	{	
		pop(i, j);
		if (!Next())
			cout << "succeed" << '\n';
	}
	break;
	case SP:
	{
		instructions.push_back(new Instruction(OPR, 0, 0));
		lv--;
		pop(i, j);
	}
	break;
	case C:
	{
		lv++;
		checklist[lv] = 0;
		if (vartable.empty())
			vartable.push_back(make_pair(new vector<pair<int, unsigned int>>(), 0));
		else if (vartable.back().second)
			vartable.push_back(make_pair(new vector<pair<int, unsigned int>>(), 0));
		pop(i, j);
		
	}
	break;
	case CDEF:
		pop(i, j);
		break;
	case INIT:
	{
		if (vartable.empty())
			vartable.push_back(make_pair(new vector<pair<int, unsigned int>>(), 0));
		else
			if (vartable.back().second)
				vartable.push_back(make_pair(new vector<pair<int, unsigned int>>(), 0));
		pair<vector<pair<int, unsigned int>>*, int> *top = &vartable.back();
		Reduced* last = buf.back();
		if (last->type != 2)
			cout << "Reducing Error: Expecting a number." << '\n';
		unsigned int intval = *((unsigned int *)last->param);
		buf.pop_back();
		buf.pop_back();
		last = buf.back();
		if (last->type != 1)
			cout << "Reducing Error: Expecting a number." << '\n';
		int index = *((int *)last->param);
		top->first->push_back(pair<int, unsigned int>(index, intval));
		buf.pop_back();
	}
		break;
	case V: 
	{
		if (vartable.back().second != 1)
			vartable.push_back(make_pair(new vector<pair<int, unsigned int>>(), 1));
		if (j == 1)
		{
			buf.pop_back();
			buf.pop_back();
			Reduced *last = buf.back();
			pair<vector<pair<int, unsigned int>>*, int> *top = &vartable.back();
			top->first->push_back(pair<int, unsigned int>(*((int *)last->param), dx[lv]++));

			vartable.push_back(make_pair(new vector<pair<int, unsigned int>>(), 2));
			propos[lv] = vartable.back().first;
			buf.pop_back();
			buf.pop_back();
		}
		INTS.push_back(vartable[vartable.size() - 2].first->size() + 2);
	}
	break;
	case VDEF:
		if (j == 1)
		{
			buf.pop_back();
			Reduced *last = buf.back();
			if (vartable.empty())
				vartable.push_back(make_pair(new vector<pair<int, unsigned int>>(), 1));
			else
				if (vartable.back().second != 1)
					vartable.push_back(make_pair(new vector<pair<int, unsigned int>>(), 1));
			pair<vector<pair<int, unsigned int>>*, int> *top = &vartable.back();
			top->first->push_back(pair<int, unsigned int>(*((int *)last->param), dx[lv]++));
			buf.pop_back();
			buf.pop_back();
		}
		break;
	case PRO:
		if (j == 1)
		{
			buf.pop_back();
			buf.pop_back();
			buf.pop_back();


			buf.pop_back();
			buf.pop_back();
		}
		break;
	case ST:
		switch (j)
		{
		case 1:
		{
			buf.pop_back();
			buf.pop_back();
			int pa = *((int *)buf.back()->param);
			buf.pop_back();
			bool found = false;
			int level = 0, d;
			searchTableforVar(pa, level, d, found);
			if (!found)
				cout << "Error: undefined identifier " << sym[pa]->value << '\n';
			else
				instructions.push_back(new Instruction(STO, level, d));
		}
		break;
		case 2:
		{
			 //JMP
			int pa = *((int *)buf.back()->param);
			buf.pop_back();
			bool found = false;
			int pos = 0;
			for  (pair<vector<pair<int, unsigned int>>*, int> var : vartable)
			{
				if (var.second == 2)
				{
					for  (pair<int, unsigned int> v : *(var.first))
					{
						if (v.first == pa)
						{
							pos = v.second;
							found = true;
							break;
						}
					}
				}
			}
			if (!found)
				cout << "Error: undefined procedure " << sym[pa]->value << '\n';
			buf.pop_back();
			instructions.push_back(new Instruction(JMP, 0, pos));
		}
			break;
		case 3:
			pop(i, j);
			break;
		case 4:
			{
				buf.pop_back();
				buf.pop_back();
				Reduced *cond = buf.back();
				instructions[((Pos *)(cond->param))->end]->rop = instructions.size();
				buf.pop_back();
				buf.pop_back();
			}
			break;
		case 5:
			{
				buf.pop_back();
				buf.pop_back();
				Reduced *cond = buf.back();
				instructions.push_back(new Instruction(JMP, 0, ((Pos *)cond->param)->start + 1));
				instructions[((Pos *)(cond->param))->end]->rop = instructions.size();
				buf.pop_back();
				buf.pop_back();
			}
			break;
		case 6:
		{
			int level = 0, d;
			bool found = false;
			buf.pop_back();

			searchTableforVar((*(int *)buf.back()->param), level, d, found);

			if (found)
			{
				instructions.push_back(new Instruction(SIO, 0, 2));
				instructions.push_back(new Instruction(STO, level, d));
			}
			else
				cout << "Error: Undefined parameter: " << sym[*(int*)buf.back()->param]->value << '\n';
			buf.pop_back();
			buf.pop_back();
			buf.pop_back();
		}
			break;
		case 7:
		{
			int level = 0, d;
			bool found = false;
			buf.pop_back();
			Reduced *id = buf.back();
			if (id->type == 1)
			{
				searchTableforVar((*(int *)(id->param)), level, d, found);
				if(found)
					instructions.push_back(new Instruction(LOD, level, d));
				else
					cout << "Error: Undefined parameter: " << sym[*(int*)buf.back()->param]->value << '\n';
			}
			else if (id->type == 2)
				instructions.push_back(new Instruction(LIT, 0, *((int *)id->param)));
			else
				cout << "Error: Undefined parameter: " << *(int*)buf.back()->param - 3 << '\n';

			instructions.push_back(new Instruction(SIO, 0, 1));
				
			buf.pop_back();
			buf.pop_back();
			buf.pop_back();
		}
			break;
		case 8:
			buf.pop_back();
		default:
			break;
		}
		break;
	case MORE:
		pop(i, j);
		break;
	case COND:
		{
			Pos *position = new Pos();
			switch (j)
			{
			case 0:
			{
				Reduced* exp = buf.back();
				position->start = *((int *) exp->param);
				instructions.push_back(new Instruction(OPR, 0, 6));
				instructions.push_back(new Instruction(JPC, 0, -1));
				buf.pop_back();
				buf.pop_back();
			}
				break;
			case 1:
			{
				Reduced* r;
				int rop = -1;
				buf.pop_back();
				r = buf.back();
				
				switch (r->type)
				{
				case P(GREATER):
					rop = 12;
						break;
				case P(LESS):
					rop = 10;
						break;
				case P(GREATEREQ):
					rop = 13;
					break;
				case P(LESSEQ):
					rop = 11;
					break;
				case P(NOTEQ):
					rop = 9;
					break;
				case P(EQUAL):
					rop = 8;
					break;
				default:
					cout << "Error: Expected an condition modifier." << '\n';
					break;
				}
				if (rop == 9)
				{
					instructions.push_back(new Instruction(OPR, 0, 8));
					instructions.push_back(new Instruction(LIT, 0, 0));
					instructions.push_back(new Instruction(OPR, 0, 8));
				}
				else
					instructions.push_back(new Instruction(OPR, 0, rop));
				
				instructions.push_back(new Instruction(JPC, 0, -1));
				buf.pop_back();
				r = buf.back();
				position->start = *((int*)r->param);
				buf.pop_back();
			}
				break;
			default:
				break;
			}
			//push
			position->end = instructions.size() - 1;
			param = (void *)position;
		}
		break;
	case EXP:
	{
		buf.pop_back();
		Pos* pos = (Pos *)buf.back()->param;
		int end = pos->end, *start = new int(pos->start);
		vector<Instruction *>::iterator iter = instructions.begin() + end;
		buf.pop_back();
		int ari2 = *((int *)buf.back()->param);
		if (ari2)
			(*iter)->rop = 1;
		else
			instructions.erase(iter);

		param = start;
		buf.pop_back();
	}
		break;
	case EXP2:
		if(j == 1)
		{
			buf.pop_back();
			int end = ((Pos *)buf.back()->param)->end;
			buf.pop_back();
			int arith = *((int *)buf.back()->param);
			if (arith)
				instructions[end]->rop = 3;
			else
				instructions[end]->rop = 2;
			buf.pop_back();
		}
		break;
	case ARITH:
	{
		buf.pop_back();
		param = new int(j);
	}
		break;
	case ARI2:
		if (j == 0)
			param = new int(0);
		else
		{
			param = new int(*((int *)buf.back()->param));
			buf.pop_back();
		}
		break;
	case TERM://head
	{
		int end = *((int *)buf.back()->param);
			buf.pop_back();
		if (end == 0)
			end = ((Pos *)buf.back()->param)->end;
		 
		int start = ((Pos *)buf.back()->param)->start;
		instructions.erase((instructions.begin() + end));
		instructions.push_back(new Instruction(OPR, 0, -2));
		buf.pop_back();
		end = instructions.size() - 1;
		Pos *pos = new Pos();
		pos->start = start;
		pos->end = end;
		param = pos;
	}
		break;
	case FACTS:
		if (j == 1)
		{
			int change = *((int*)buf.back()->param);//FACTS.end

			buf.pop_back();
			int end = ((Pos *)buf.back()->param)->end;//fact->end
			buf.pop_back();
			int muldiv = *((int *)buf.back()->param);
			if (muldiv)
				instructions[end]->rop = 5;
			else
				instructions[end]->rop = 4;
			param = new int((!change) ? change : end);
			buf.pop_back();
		}
		break;
	case MULDIV:
		{
			buf.pop_back();
			param = new int (j);
		}
		break;
	case FACT:
	{
		Pos* pos = new Pos();
		switch (j) {
		case 0:
		{
			int level = 0, d;
			bool found = false;
			pos->start = instructions.size() - 1;
			bool type = searchTableforVar(*((int *)buf.back()->param), level, d, found, true);
			
			buf.pop_back();
			if (found && type)
				instructions.push_back(new Instruction(LOD, level, d));
			else if(found && !type)
				instructions.push_back(new Instruction(LIT, 0, d));
			else
				cout << "Error: Unfefined identifier: " << sym[*((int *)buf.back()->param)]->value << '\n';
		}
		break;
		case 1:
		{
			pos->start = instructions.size() - 1;
			instructions.push_back(new Instruction(LIT, 0, *((int *)buf.back()->param)));
			buf.pop_back();
		}
		break;
		case 2:
		{
			buf.pop_back();
			Reduced *exp = buf.back();
			pos->start = *((int *)exp->param);
			buf.pop_back();
			buf.pop_back();
		}
		break;
		default:
			break;
		}
		instructions.push_back(new Instruction(OPR, 0, -1));
		pos->end = instructions.size() - 1;
		param = (void *)pos;
	}
		break;
	
	}
	return new Reduced(S(i), param);
} 


void initStates() {

	states = new int**[FACT + 1];	
	states[P] = init_st(3, S(SP), P(DOT), -1);
	states[SP] = init_st(6, S(C), S(V), S(PRO), S(ST), P(SEMICOLON), -1);
	states[C] = init_st(6, -2, P(CONST), S(INIT), S(CDEF), P(SEMICOLON), -1);
	states[CDEF] = init_st(5, -2, P(COMMA), S(INIT), S(CDEF), -1);
	states[INIT] = init_st(4, 1, P(EQUAL), 2, -1);
	states[V] = init_st(6, -2, P(VAR), 1, S(VDEF), P(SEMICOLON), -1);
	states[VDEF] = init_st(5, -2, P(COMMA), 1, S(VDEF), -1);
	states[PRO] = init_st(7, -2, P(PROCEDURE), 1, P(SEMICOLON), S(SP), S(PRO), -1);
	states[ST] = init_st(35, -2, 1, P(ASSIGNMENT), S(EXP), -2, P(CALL), 1, -2,
		P(BEGIN), S(ST), S(MORE), P(END), -2, P(IF), S(COND), P(THEN), S(ST), -2,
		P(WHILE), S(COND), P(DO), S(ST), -2, P(READ), P(LP), 1, P(RP), -2, P(WRITE),
		P(LP), 1, P(RP), -2, P(SEMICOLON), - 1);
	states[MORE] = init_st(5, -2, P(SEMICOLON), S(ST), S(MORE), -1);
	states[COND] = init_st(7, P(ODD), S(EXP), -2, S(EXP), A_MOD, S(EXP), -1);
	states[EXP] = init_st(4, S(ARI2), S(TERM), S(EXP2), -1);
	states[EXP2] = init_st(5, -2, S(ARITH), S(TERM), S(EXP2), -1);
	states[ARITH] = init_st(4, P(PLUS), -2, P(MINUS), -1);
	states[ARI2] = init_st(3, -2, S(ARITH), - 1);
	states[TERM] = init_st(3, S(FACT), S(FACTS), -1);
	states[FACTS] = init_st(5, -2, S(MULDIV), S(FACT), S(FACTS), -1);
	states[MULDIV] = init_st(4, P(MULTIPLE), -2, P(DIVISION), -1);
	states[FACT] = init_st(8, 1, -2, 2, -2, P(LP), S(EXP), P(RP), -1);
	
	follows = new int*[FACT + 1];
	follows[P] = init_follow(1, -1);
	follows[SP] = init_follow(11, P(DOT), P(END), P(SEMICOLON), P(BEGIN), P(CALL), P(IF), P(WHILE), P(READ), P(WRITE), 1, P(PROCEDURE));
	follows[C] = init_follow(11, P(VAR), P(END), P(SEMICOLON), P(BEGIN), P(CALL), P(IF), P(WHILE), P(READ), P(WRITE), 1, P(PROCEDURE));
	follows[CDEF] = init_follow(1, P(SEMICOLON));
	follows[INIT] = init_follow(2, P(SEMICOLON), P(COMMA));
	follows[V] = init_follow(10, P(SEMICOLON), P(PROCEDURE), P(END), P(BEGIN), P(CALL), P(IF), P(WHILE), P(READ), P(WRITE), 1);
	follows[VDEF] = init_follow(1, P(SEMICOLON));
	follows[PRO] = init_follow(9, P(END), P(SEMICOLON), P(BEGIN), P(CALL), P(IF), P(WHILE), P(READ), P(WRITE), 1);
	follows[ST] = init_follow(2, P(END), P(SEMICOLON));
	follows[MORE] = init_follow(1, P(END));
	follows[COND] = init_follow(2, P(THEN), P(DO));
	follows[EXP] = init_follow(6, A_MOD, P(THEN), P(DO), P(RP), P(END), P(SEMICOLON));
	follows[EXP2] = init_follow(6, A_MOD, P(THEN), P(DO), P(RP), P(END), P(SEMICOLON));
	follows[ARITH] = init_follow(3, 1, 2, P(LP));
	follows[ARI2] = init_follow(3, 1, 2, P(LP));
	follows[TERM] = init_follow(8, P(PLUS), P(MINUS), A_MOD, P(THEN), P(DO), P(RP), P(END), P(SEMICOLON));
	follows[FACTS] = init_follow(8, P(PLUS), P(MINUS), A_MOD, P(THEN), P(DO), P(RP), P(END), P(SEMICOLON));
	follows[MULDIV] = init_follow(3, 1, 2, P(LP));
	follows[FACT] = init_follow(10, P(MULTIPLE), P(DIVISION), P(PLUS), P(MINUS), A_MOD, P(THEN), P(DO), P(END), P(SEMICOLON), P(RP));


	instructions.pb(new Instruction(JMP, 0, -1));
	firstjump = instructions[0];
	checklist[0] = true;

}



inline void expend(set<Indexes> &togo) {

	int size0, j;
	do {
		size0 = togo.size();
		for (Indexes var : togo)
		{
			j = states[var.i][var.j][var.k];
			int i;
			if (_S(j))
				for (i = 0; i < 16; i++) {
					if (states[DeS(j)][i][0] == -3)
						break;
					else
						togo.insert(Indexes(DeS(j), i, 0));
				}
		}
	} while (size0 != togo.size());

}

void Shift(set<Indexes> togo) {
	if (lv == 0)
		int null = 1;

	//shifting
	Reduced* nextsym = peekNext();
	set<Indexes> tobeReduced;
	int nexttype = is_not_mod(nextsym->type), t;
	archive.push(togo);
	set<Indexes>::pointer pt;
	set<Indexes>::iterator var = togo.begin();
	while(var != togo.end()){
 		t = states[var->i][var->j][var->k];
		if (t < 0)
		{
			tobeReduced.insert(Indexes(var->i, var->j, 0));
			var = togo.erase(var);
			if (togo.empty())
				break;
		}
		else if (t != nexttype)
		{
			var = togo.erase(var);
			if (togo.empty())
				break;
		}
		else
			var++;
	}
	//reducing 
	int FOL;
	bool match;
	int j;
	var = tobeReduced.begin();
	//reduce conflict.
	while (var != tobeReduced.end())
	{
		j = 0;
		match = false;
		FOL = follows[var->i][j++];
		while (FOL > 0)
			if (is_not_mod(nextsym->type) == FOL)
			{
				match = true;
				break;
			}
			else
				FOL = follows[var->i][j++];

		if (!match)
			var = tobeReduced.erase(var);
		else
			var++;

		if (tobeReduced.empty())
			break;
		
	}
	if (tobeReduced.size() > 0)
	{
		int i = tobeReduced.begin()->i, k = 0;
		j = tobeReduced.begin()->j;
		nextSym = reduce(i, j);
		
		while (states[i][j][k++] > 0)
			archive.pop();
		set<Indexes> togoNext = archive.top();
		archive.pop();
		Shift(togoNext);
		return;
	}
	else if (Next())
	{
		if (!togo.empty())
		{
			set<Indexes> togoNext;
			for (Indexes var : togo){
				var.k++;
				togoNext.insert(var);
			}
			togo.clear();
			
			expend(togoNext);
			buf.push_back(nextsym);
			if (nexttype == P(PROCEDURE) && !geting_proc)
				geting_proc = true;
			else if (geting_proc && nexttype == 1)
			{
				propos[lv]->push_back(pair<int, unsigned int>(*((int *)nextsym->param), numeric_limits<int>::max()));
				checklist[lv] = false;
				geting_proc = false;
			}

			Shift(togoNext);
		}
	}
}

void BLOCK() {
	initStates();
	it = identifiers.begin();
	set<Indexes> going;
	going.insert(Indexes(P, 0, 0));

	//
	expend(going);
	Shift(going);


	vartable.clear();
	cout << address <<Next() << '\n';
}

vector<void **> *memory;
vector<int> *t;

int  PC, I;
void* b;
#define getij() _getij(i, j)
#define geti _geti(i)
inline void _getij(int &i, int &j) {
	j = t->back();
	t->pop_back();
	i = t->back();
	t->pop_back();
}
inline void _geti(int &i) {
	i = t->back();
	t->pop_back();
}
void PARSE() {

	int inst = 0;
	for (auto var : instructions)
		cout << inst++ << " " << operations[var->nop] << " " << var->lop << " " << var->rop << '\n';
	
	I = 0;
	PC = -1;
	int i, j;
	memory = new vector<void **>;
	while (I < instructions.size())
	{

		cout << "\n inst "<< I << '\n';
		switch (instructions[I] ->nop)
		{
		case INT:
		{
			b = new void*[instructions[I]->rop];
			memory->push_back((void **)b);

			t = new vector<int>;
			((void **)b)[0] = t;
			((void **)b)[1] = b;
			((void **)b)[2] = new int(PC);
		}
			break;
		case OPR:
			switch (instructions[I]->rop)
			{
			case 0:
			{
				if ((*(int *)(((void**)b)[2])) > 0)
					I = (*(int *)(((void**)b)[2]));
				else
					return;
				
				delete (vector< int> *)((void **)b)[0];
				delete[] ((void**)b);
				memory->pop_back();
				t = (vector< int > *)(memory->back()[0]);
				b = memory->back()[1];
				continue;
			}	
			break;
			case 1:
			{
				geti;
				t->push_back(-i);
			}
				break;
			case 2:
			{
				getij();
				t->push_back(i + j);
			}
				break;
			case 3:
			{
				getij();
				t->push_back(i - j);
			}
				break;
			case 4:
			{
				getij();
				t->push_back(i * j);
			}
			break;
			case 5:
			{
				getij();
				if (j == 0)
				{
					cout << "Error: Division by zero" << '\n';
					exit(1);
				}
				t->push_back(i / j);
			}
			break;
			case 6:
			{
				geti;
				t->push_back((i % 2) != 0);
			}
			break;
			case 8:
			{
				getij();
				t->push_back(i == j);
			}
			break;
			case 9:
			{
				getij();
				t->push_back(i != j);
			}
			break;
			case 10:
			{
				getij();
				t->push_back(i < j);
			}
			break;
			case 11:
			{
				getij();
				t->push_back(i <= j);
			}
			break;
			case 12:
			{
				getij();
				t->push_back(i > j);
			}
			break;
			case 13:
			{
				getij();
				t->push_back(i >= j);
			}

			break;
			}
			break;
		case SIO:
			if (instructions[I]->rop == 2) {
				cin >> i;
				t->push_back(i);
			}
			else {
				i = t->back();
				t->pop_back();
				cout << i <<'\n';

			}
			break;
		case LOD:
		{
			i = *(int *)(*memory)[instructions[I]->lop][instructions[I]->rop + 3];
			t->push_back(i);
		} 
			break;
		case LIT:
			t->push_back(instructions[I]->rop); 
			break;
		case JMP:
			I = instructions[I]->rop;
			continue;
			break;
		case JPC:
		{
			geti;
			if (!i)
			{
				I = instructions[I]->rop;
				continue;
			}
		}	break;
		case STO:
		{
			geti;
			(*memory)[instructions[I]->lop][instructions[I]->rop + 3] = new int(i);
		}
			break;
		case CAL:
			I = instructions[I]->rop;
			break;
		default:
			cout << "Undefined Instruction: \n" << operations[instructions[I]->nop] << " " << instructions[I]->lop << " " << instructions[I]->rop << '\n';
			break;
		}
		I++;
		PC = I + 1;
	}
	delete memory;
}


int main(int argc, char** argv)
{

	long length;
	char* file = 0;
	FILE *fp = NULL;
	if (argc > 1) 
		fp = fopen(argv[1], "r");
	else
		fp = fopen("input.pas", "r");

	if (!fp)
		file = new char[65536]; 
loop_input:	while (!fp) {
		cout << "Input the path of input file."<<'\n';
		scanf("%s", file);
		fp = fopen(file, "r");
	}
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	if (length > MAX_BUFFER)
	{
		cout << "Cannot handle now:)" << '\n';
		fclose(fp);
		fp = NULL;
		goto loop_input;
	}
	fseek(fp, 0, SEEK_SET);
	fread(buffer, 1, length, fp);
	fclose(fp);

	buffer[length] = ' ';
	buffer[length] = 0;
	sym.reserve(512);
	num.reserve(512);
	
	//coping
	GETSYM();
	//PARSING
	BLOCK();
	//Interprating
	PARSE();

	cout << "\n\nProgram ended.\n";
	if(file)
		delete[] file;
	return 0;
}

