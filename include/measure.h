/**
 * @file read-measure.h
 * @brief Read the measure from the sensor and save it on the file
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @date 2023-07-10
 * @version 1.0.0
 */
#ifndef _Measure_h_
#define _Measure_h_

#include <config/file-system.h>
#include <file.h>

#include <StringHelper.h>

/**
 * @brief The measure of a sensor
 */
struct Measure
{
    String value;
    String idType;
};

/**
 * @brief Save the measure on the file
 *
 * @param measures the measures to save
 *
 * @return ErrorOr<void>
 */
auto SaveMeasureOnFile(const LL<Measure> &measures) -> ErrorOr<void>
{
    ErrorOr<> result;

    if (!FileExists(MEASURE_FILE))
    {
        CreateFile(MEASURE_FILE);
    }

    auto openResult = OpenFile(MEASURE_FILE, "w");

    if (!openResult.ok())
    {
        result = failure(openResult.error());
    }
    else
    {
        auto file = openResult.unwrap();

        for (auto measure : measures)
        {
            auto line = String(measure.value) + ";" + measure.idType;
            file.println(line);
        }

        file.close();
    }

    return result;
}

/**
 * @brief Read the measure from the file
 *
 * @return ErrorOr<LL<Measure>> list of measures.
 */
auto ReadMeasureFromFile() -> ErrorOr<LL<Measure>>
{
    ErrorOr<LL<Measure>> result;

    if (IsEmptyFile(MEASURE_FILE))
    {
        result = failure({
            .context = "ReadMeasureFromFile",
            .message = "File is empty",
        });
    }
    else
    {
        auto readResult = ReadFromFile(MEASURE_FILE, '\n');

        if (!readResult.ok())
        {
            result = failure(readResult.error());
        }
        else
        {
            auto lines = readResult.unwrap();
            auto measures = LL<Measure>();

            for (auto line : lines)
            {
                auto splitResult = utility::StringHelper::splitStringToArray(line, ';');

                if (splitResult.ok())
                {
                    auto array = splitResult.unwrap();
                    auto value = array.at(0)->substring(0, array.at(0)->length() - 1);
                    auto idType = array.at(1)->substring(1, array.at(1)->length() - 1);

                    measures.add({
                        .value = value,
                        .idType = idType,
                    });
                }
            }

            result = ok(measures);
        }
    }

    return result;
}

/**
 * @brief Convert the list of measures to string
 *
 * @param measures the list of measures
 *
 * @return String the string
 */
auto ListOfMeasureToString(const LL<Measure> &measures) -> String
{
    String result = "";

    for (auto measure : measures)
    {
        result += measure.value + "," + measure.idType + ";";
    }

    return result;
}

#endif // ! _Measure_h_
