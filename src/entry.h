#ifndef ENTRY_H
#define ENTRY_H

#include <iostream>
#include<vector>

class Entry {
public:
	
	Entry(unsigned int r, int m, std::vector<float> e, int q);
	
	// For blank entries.
	Entry(unsigned int r, int m);

	unsigned int get_read_id() const;
	int get_allele_type() const;
	std::vector<float> get_emission_score() const;
	int get_quality() const;

	void set_read_id(unsigned int r);
	void set_allele_type(int m);
	void set_emission_score(std::vector<float> e);
	void set_quality(int q);

	friend std::ostream& operator<<(std::ostream& out, const Entry& e);

private:
	unsigned int read_id;
	int allele;
	std::vector<float> emission_score;
	int quality;
};

#endif
