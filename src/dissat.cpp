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
    // [shift][pipeline]
    std::vector<std::vector<long double>> pipeline_shift_dissat;
    std::vector<long double> generate_shift_dissat();
    void generate_dissat();
    WeightedDissat(const std::string &dataset_code);
    // schedule: [worker][shift][pipeline][job]
    std::vector<long double> calculate_dissat(const std::vector<std::vector<std::vector<std::vector<int>>>> &schedule);
};

std::vector<long double> dissat::WeightedDissat::generate_shift_dissat()
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
void dissat::WeightedDissat::generate_dissat()
{
    std::vector<long double> shift_dissat = generate_shift_dissat();
    const std::vector<std::vector<int>> &working_hours = dataset.pipeline_time;
    pipeline_shift_dissat.assign(working_hours.size(),
                                 std::vector<long double>(working_hours[0].size()));
    for (size_t shift = 0; shift < working_hours.size(); shift++)
        for (size_t pipeline = 0; pipeline < working_hours[0].size(); pipeline++)
            pipeline_shift_dissat[shift][pipeline] = shift_dissat[shift] * working_hours[shift][pipeline];
}
std::vector<long double> dissat::WeightedDissat::calculate_dissat(
    const std::vector<std::vector<std::vector<std::vector<int>>>> &schedule)
{
    std::vector<long double> dissat(schedule.size());
    for (size_t worker = 0; worker < schedule.size(); worker++)
        for (size_t shift = 0; shift < schedule[0].size(); shift++)
            for (size_t pipeline = 0; pipeline < schedule[0][0].size(); pipeline++)
                for (size_t job = 0; job < schedule[0][0][0].size(); job++)
                    dissat[worker] += pipeline_shift_dissat[shift][pipeline] *
                                      schedule[worker][shift][pipeline][job];
    return dissat;
}
dissat::WeightedDissat::WeightedDissat(const std::string &dataset_code) : dataset(dataset_code),
                                                                          size(dataset.pipeline_time.size())
{
    generate_dissat();
}
