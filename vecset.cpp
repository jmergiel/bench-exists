#include <cstdio>
#include <chrono>
#include <vector>
#include <set>
#include <numeric>
#include <cassert>
#include <map>
#include <string>
#include <unordered_set>

// benchmark name format
#define ID_FMT "[%40s]"

typedef uint64_t val_t;					//< the type we store in containers & look for
typedef std::vector<val_t> vec_t;
using namespace std::chrono;			//< it's too long to leave that everywhere
typedef high_resolution_clock ClockT;	//< the clock we're using
typedef nanoseconds unit_t;				//< the unit in which we measure (stored in uint64_t)

namespace
{
	static std::multimap<uint64_t, const std::string> cmp;

	// prints relative comparsion - all times versus the fastest (ordered)
	void PrintComparison()
	{
		printf("\nRelative comparison (= slower than the fastest by this ratio):\n");
		const double base = cmp.cbegin()->first;
		for(auto i : cmp) printf(ID_FMT " %.03fx\n", i.second.c_str(), float(i.first/base));
		cmp.clear();
	}
	
	// prints intermediate results (as they are measured)
	void PrintResult(uint64_t ns, size_t loops, const char* name)
	{
		cmp.emplace(ns, name);
		printf(ID_FMT " %lu(ns) %.06f(ns/loop)\n", name, ns, ns/float(loops));
	}
}

namespace Vector
{
	inline bool IsInVectorIdxSize(const vec_t& vec, val_t val)
	{
		for(size_t i=0; i<vec.size(); ++i) if(vec[i] == val) return true;
		return false;
	}
	inline bool IsInVectorIdxNoSize(const vec_t& vec, val_t val)
	{
		const size_t Size = vec.size();
		for(size_t i=0; i<Size; ++i) if(vec[i] == val) return true;
		return false;
	}
	inline bool IsInVectorIterEnd(const vec_t& vec, val_t val)
	{
		for(vec_t::const_iterator it = vec.begin(); it != vec.end(); ++it) if(*it==val) return true;
		return false;
	}
	inline bool IsInVectorIterNoEnd(const vec_t& vec, val_t val)
	{
		const vec_t::const_iterator End = vec.end();
		for(vec_t::const_iterator it = vec.begin(); it != End; ++it) if(*it==val) return true;
		return false;
	}

	void TestVector(const size_t elems, const size_t loops, const size_t expected_count, val_t needle)
	{
		vec_t vec(elems);
		std::iota(vec.begin(), vec.end(), 1);

		printf("\nSearching for %s element in vector:\n", needle ? "middle" : "non-existent");
		{
			size_t counter = 0;
			const time_point<ClockT> start = ClockT::now();
			for(size_t L=0; L<loops; ++L) if(IsInVectorIdxSize(vec, needle)) ++counter;
			const time_point<ClockT> end = ClockT::now();
			assert(counter == expected_count);
			PrintResult(duration_cast<unit_t>(end - start).count(), loops, "vector, indexed by size_t, using size()");
		}
		{
			size_t counter = 0;
			const time_point<ClockT> start = ClockT::now();
			for(size_t L=0; L<loops; ++L) if(IsInVectorIdxNoSize(vec, needle)) ++counter;
			const time_point<ClockT> end = ClockT::now();
			assert(counter == expected_count);
			PrintResult(duration_cast<unit_t>(end - start).count(), loops, "vector, indexed by size_t, cached size()");
		}
		{
			size_t counter = 0;
			const time_point<ClockT> start = ClockT::now();
			for(size_t L=0; L<loops; ++L) if(IsInVectorIterEnd(vec, needle)) ++counter;
			const time_point<ClockT> end = ClockT::now();
			assert(counter == expected_count);
			PrintResult(duration_cast<unit_t>(end - start).count(), loops, "vector, iterated, calling end()");
		}
		{
			size_t counter = 0;
			const time_point<ClockT> start = ClockT::now();
			for(size_t L=0; L<loops; ++L) if(IsInVectorIterNoEnd(vec, needle)) ++counter;
			const time_point<ClockT> end = ClockT::now();
			assert(counter == expected_count);
			PrintResult(duration_cast<unit_t>(end - start).count(), loops, "vector, iterated, cached end()");
		}
	}
}

namespace Sets
{
	template <typename SetT>
	inline bool IsInSetCount(const SetT& st, val_t val)
	{
		return st.count(val);
	}
	template <typename SetT>
	inline bool IsInSetFind(const SetT& st, val_t val)
	{
		return st.find(val) != st.end();
	}

	template<typename SetT>
	void TestSet(const size_t elems, const size_t loops, const size_t expected_count, const char* name, val_t needle)
	{
		typedef SetT set_t;
		set_t set;
		for(size_t i=1; i<=elems; ++i) set.emplace(i);	//< no easy std::iota for set

		std::string Name(name);
		printf("\nSearching for %s element in %s:\n", needle ? "middle" : "non-existent", name);
		{
			size_t counter = 0;
			const time_point<ClockT> start = ClockT::now();
			for(size_t L=0; L<loops; ++L) if(IsInSetCount(set, needle)) ++counter;
			const time_point<ClockT> end = ClockT::now();
			assert(counter == expected_count);
			PrintResult(duration_cast<unit_t>(end - start).count(), loops, (Name+", using count()").c_str());
		}
		{
			size_t counter = 0;
			const time_point<ClockT> start = ClockT::now();
			for(size_t L=0; L<loops; ++L) if(IsInSetFind(set, needle)) ++counter;
			const time_point<ClockT> end = ClockT::now();
			assert(counter == expected_count);
			PrintResult(duration_cast<unit_t>(end - start).count(), loops, (Name+", using find()").c_str());
		}
	}
}

int main(int argc, const char** argv)
{
	if(argc != 3)
	{
		printf("This benchmark initializes a container with consecutive numbers and looks for a single value\n");
		printf("using different methods. Benchmarked containers: std::vector, std::set and std::unordered_set\n");
		printf("This is all about measuring the time needed to check if an element exists, nothing else matters.\n");
		printf("\nUSAGE: number_of_elements loops\n\n");
		return -1;
	}

	const size_t elems = std::stoul(argv[1]);
	const size_t loops = std::stoul(argv[2]);

	printf("Benchmark with %lu elements (data size per test = %.03fMB) and %lu loops\n", elems, (sizeof(val_t)*elems)/1024.0f/1024.0f, loops);

	// i==0: search for non-existent value
	// i==1: search for middle value
	for(int i=0; i<2; ++i)
	{
		const size_t exp = i ? loops : 0;		//< expected count of finds
		const val_t needle = i ? elems/2 : 0;	//< value to search for
		Vector::TestVector(elems, loops, exp, needle);
		Sets::TestSet<std::set<val_t> >(elems, loops, exp, "set", needle);
		Sets::TestSet<std::unordered_set<val_t> >(elems, loops, exp, "unordered_set", needle);
		PrintComparison();
	}

	return 0;
}
