#include <vector>
#include <string>

#include "input.cpp"

namespace dissat
{
    class WeightedDissat;
}

class dissat::WeightedDissat
{
    size_t size;
    input::dataset dataset;
    std::vector<long double> generate_shift_dissat()
    {
        std::vector<long double> shift_dissat(size, 1);
        // For night shifts, increase dissat by 50%. There are 3 shifts per day
        for (size_t i = 2; i < size; i += 3)
            shift_dissat[i] *= 1.5;
        // For Saturday shifts, increase dissat by 10%.
        for (size_t shift = 6; shift <= 8; shift++)
            for (size_t i = shift; i < size; i += 21)
                shift_dissat[i] *= 1.1;
        // For Sunday shifts, increase dissat by 70%.
        for (size_t shift = 9; shift <= 11; shift++)
            for (size_t i = shift; i < size; i += 21)
                shift_dissat[i] *= 1.7;
        return shift_dissat;
    }
    // [pipeline][shift]
    std::vector<std::vector<long double>> generate_pipeline_shift_dissat()
    {
        std::vector<long double> shift_dissat = generate_shift_dissat();
        const std::vector<std::vector<int>> &working_hours = dataset.pipeline_time;
        std::vector<std::vector<long double>> pipeline_shift_dissat(
            working_hours.size(), std::vector<long double>(size));
        for (size_t pipeline = 0; pipeline < working_hours.size(); pipeline++)
            for (size_t shift = 0; shift < size; shift++)
                pipeline_shift_dissat[pipeline][shift] = shift_dissat[shift] * (working_hours[pipeline][shift] + 2);
        return pipeline_shift_dissat;
    }
    WeightedDissat(const std::string &dataset_code) : dataset(dataset_code), size(dataset.pipeline_time[0].size())
    {
        std::vector<std::vector<long double>> pipeline_shift_dissat = generate_pipeline_shift_dissat();
    }
};
