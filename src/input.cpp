#include <string>
#include <algorithm>
#include <map>
#include <utility>
#include <cassert>
#include <vector>
#include <fstream>

namespace input
{
    int convert_timestamps(const std::string &timestamp)
    {
        // 2023-06-01 08:00:00
        int day = (timestamp[8] - '0') * 10 + timestamp[9] - '0' - 1;
        int hour = (timestamp[11] - '0') * 10 + timestamp[12] - '0';
        return day * 24 - hour;
    }
    template <typename T>
    std::vector<T> eliminate_duplicates(std::vector<T> values)
    {
        std::sort(values.begin(), values.end());
        typename std::vector<T>::iterator last_it = std::unique(
            values.begin(), values.end());
        size_t tsize = last_it - values.begin();
        values.resize(tsize);
        return values;
    }
    class dataset;
}
class input::dataset
{
    std::string dataset_code;
    std::vector<std::vector<int>> pipeline_cardi, pipeline_time;
    std::vector<std::string> employee_codes, pipeline_codes, job_codes;
    std::vector<std::vector<std::vector<int>>> pipeline_candidates;

    void retrieve_workers();
    void retrieve_variables();
    void retrieve_skills();
    std::vector<int> retrieve_single_skill(const std::string &pipeline, const std::string &job);
    void retrieve_pipeline_time();
    std::vector<int> retrieve_single_pipeline_shift(const std::string &pipeline);
    void add_pipeline_time(std::vector<int> &current_pipeline, int start, int end);
    dataset(std::string &dataset_code);
};

input::dataset::dataset(std::string &dataset_code) : dataset_code(dataset_code)
{
    retrieve_workers();
    retrieve_variables();
    retrieve_skills();
}

void input::dataset::retrieve_workers()
{
    char fpath[1000];
    sprintf(fpath, "data/%s/01_nhan_su.txt", dataset_code.c_str());
    std::ifstream input(fpath);

    std::string header;
    std::getline(input, header);
    assert(header == "# so_thu_tu ma_nhan_su");

    int id;
    std::string value;
    while (input >> id >> value)
    {
        employee_codes.push_back(value);
        assert(id == employee_codes.size());
    }
    std::sort(employee_codes.begin(), employee_codes.end());

    input.close();
}

void input::dataset::retrieve_variables()
{
    char fpath[1000];
    sprintf(fpath, "data/%s/02_dinh_bien.txt", dataset_code.c_str());
    std::ifstream input(fpath);

    std::vector<std::string> pipeline_list, job_list;
    std::vector<std::tuple<std::string, std::string, int>> data;

    int requirement;
    std::string pipeline, job;
    while (input >> pipeline >> job >> requirement)
    {
        data.emplace_back(pipeline, job, requirement);
        pipeline_list.push_back(pipeline);
        job_list.push_back(job);
    }

    pipeline_codes = eliminate_duplicates(pipeline_list);
    job_codes = eliminate_duplicates(job_list);

    pipeline_cardi.assign(pipeline_codes.size(),
                          std::vector<int>(job_codes.size()));
    for (auto [pipeline, job, requirement] : data)
    {
        int pipeline_id = std::lower_bound(
                              pipeline_codes.begin(), pipeline_codes.end(), pipeline) -
                          pipeline_codes.begin();
        int job_id = std::lower_bound(
                         job_codes.begin(), job_codes.end(), job) -
                     job_codes.begin();
        pipeline_cardi[pipeline_id][job_id] = requirement;
    }

    input.close();
}

void input::dataset::retrieve_skills()
{
    pipeline_candidates.assign(pipeline_codes.size(),
                               std::vector<std::vector<int>>(job_codes.size()));
    for (size_t pipeline_id = 0; pipeline_id < pipeline_codes.size(); pipeline_id++)
    {
        const std::string &pipeline = pipeline_codes[pipeline_id];
        for (size_t job_id = 0; job_id < job_codes.size(); job_id++)
        {
            const std::string &job = job_codes[job_id];
            pipeline_candidates[pipeline_id][job_id] = retrieve_single_skill(pipeline, job);
        }
    }
}

std::vector<int> input::dataset::retrieve_single_skill(const std::string &pipeline, const std::string &job)
{
    char fpath[1000];
    sprintf(fpath, "data/%s/ky_nang_%s_%s.txt",
            dataset_code.c_str(), pipeline.c_str(), job.c_str());
    std::ifstream input(fpath);

    std::vector<int> result;
    std::string employee_code;
    while (input >> employee_code)
    {
        int employee_id = std::lower_bound(
                              employee_codes.begin(), employee_codes.end(), employee_code) -
                          employee_codes.begin();
        result.push_back(employee_id);
    }

    input.close();
    return result;
}

void input::dataset::retrieve_pipeline_time()
{
    pipeline_time.assign(pipeline_codes.size(), std::vector<int>());
    for (size_t pipeline_id = 0; pipeline_id < pipeline_codes.size(); pipeline_id++)
    {
        const std::string &pipeline = pipeline_codes[pipeline_id];
        pipeline_time[pipeline_id] = retrieve_single_pipeline_shift(pipeline);
    }
}

std::vector<int> input::dataset::retrieve_single_pipeline_shift(const std::string &pipeline)
{
    char fpath[1000];
    sprintf(fpath, "data/%s/lenh_san_xuat_%s.txt",
            dataset_code.c_str(), pipeline.c_str());
    std::ifstream input(fpath);

    std::string line;
    getline(input, line);
    assert(line == "# Thoi_gian_bat_dau Thoi_gian_ket_thuc");

    std::vector<int> current_pipeline;
    while (getline(input, line))
    {
        int start_timestamp = convert_timestamps(line.substr(0, 19));
        int end_timestamp = convert_timestamps(line.substr(21, 19));
        add_pipeline_time(current_pipeline, start_timestamp, end_timestamp);
    }

    input.close();
    return current_pipeline;
}

// There are three shifts: 6-14, 14-22, 22-6
void input::dataset::add_pipeline_time(std::vector<int> &current_pipeline, int start, int end)
{
    int starting_block = start / 8;
    int ending_block = (end - 1) / 8;
    current_pipeline.resize(std::max(current_pipeline.size(), size_t(ending_block) + 1));
    for (int block = starting_block; block <= ending_block; block++)
    {
        int start_of_block = block * 8, end_of_block = (block + 1) * 8;
        current_pipeline[block] += std::min({8, end - start_of_block, end_of_block - start});
    }
}
