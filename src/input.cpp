#include <string>
#include <algorithm>
#include <map>
#include <utility>
#include <cassert>
#include <vector>
#include <fstream>

namespace input
{
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
    class dataset
    {
        std::string dataset_code;
        std::vector<std::vector<int>> pipeline_requirements;
        std::vector<std::string> employee_codes, pipeline_codes, job_codes;
        std::vector<std::vector<std::vector<int>>> pipeline_candidates;

        void get_workers();
        void get_variables();
        void get_skills();
        std::vector<int> get_single_skill(const std::string &pipeline, const std::string &job);
        dataset(std::string &dataset_code) : dataset_code(dataset_code)
        {
            get_workers();
            get_variables();
            get_skills();
        }
    };
}

void input::dataset::get_workers()
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

void input::dataset::get_variables()
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

    pipeline_requirements.assign(pipeline_codes.size(),
                                 std::vector<int>(job_codes.size()));
    for (auto [pipeline, job, requirement] : data)
    {
        int pipeline_id = std::lower_bound(
                              pipeline_codes.begin(), pipeline_codes.end(), pipeline) -
                          pipeline_codes.begin();
        int job_id = std::lower_bound(
                         job_codes.begin(), job_codes.end(), job) -
                     job_codes.begin();
        pipeline_requirements[pipeline_id][job_id] = requirement;
    }

    input.close();
}

void input::dataset::get_skills()
{
    pipeline_candidates.assign(pipeline_codes.size(),
        std::vector<std::vector<int>> (job_codes.size()));
    for (size_t pipeline_id = 0; pipeline_id < pipeline_codes.size(); pipeline_id++)
    {
        const std::string &pipeline = pipeline_codes[pipeline_id];
        for (size_t job_id = 0; job_id < job_codes.size(); job_id++)
        {
            const std::string &job = job_codes[job_id];
            pipeline_candidates[pipeline_id][job_id] = get_single_skill(pipeline, job);
        }
    }
}

std::vector<int> input::dataset::get_single_skill(const std::string &pipeline, const std::string &job)
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
