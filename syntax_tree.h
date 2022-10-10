#ifndef SYNTAX_TREE_HPP_DEFINED
#define SYNTAX_TREE_HPP_DEFINED

#include <map>

/***********************************************************************************************
* a helping class top parse text tokens
************************************************************************************************/

template<class C,class T>
class SyntaxTree
{
public:

	static constexpr uint32_t NoIndex=(uint32_t)-1;

	struct Init
	{
		const C* pToken;	// Token can contain upper lower case
		T        Value;		// the value for this token
	};

	SyntaxTree(Init* pInit)
	{
		uint32_t Index=0;
		while (pInit->pToken)
			m_Branch.Feed((pInit++)->pToken, Index++);
	}

	uint32_t Find(const C* pToken)
	{
		return m_Branch.Find(pToken);
	}

private:

	class Branch: private std::map<C,Branch*>
	{
	public:

		typedef std::map<C,Branch*> BaseClass;

		uint32_t m_Index; // index into original table of Init elements

		Branch(uint32_t Index = NoIndex): m_Index(Index) {	}
		~Branch() { for(auto i : *this)	delete i.second; }

		void Feed(const C* pToken,uint32_t Index)
		{
			C key=toupper(*pToken++);
			if(key)
			{
				auto i=BaseClass::find(key);
				Branch* pSubBranch=nullptr;
				if(i==BaseClass::end())
					BaseClass::insert({key,(pSubBranch=new Branch(Index))});
				else
					pSubBranch=i->second;
				pSubBranch->Feed(pToken,Index);
			}
		}

		uint32_t Find(const C* pToken)
		{
			C key=toupper(*pToken++);
			if (key)
			{
				auto i=BaseClass::find(key);
				if(i!=BaseClass::end())
					return i->second->Find(pToken);
				else
					return NoIndex;
			}
			return m_Index;
		}

	};

	Branch m_Branch; // entry point for token tree
};


#endif