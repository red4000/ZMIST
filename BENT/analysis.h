#ifndef ANALYSIS_H
#define ANALYSIS_H

class AnalysisContext {
public:
	AnalysisContext();
	~AnalysisContext();

	void IterateBytes();
	int MarkBlock(size_t rva);
	int AnalyzeBlock(size_t rva);
	int AnalyzeNext();
	int Analyze(Bent *_b);

	Bent          *b;
	Vector<size_t> next, bNext, cref;
};

#endif
