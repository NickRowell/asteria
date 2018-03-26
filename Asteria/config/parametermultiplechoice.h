#ifndef PARAMETERMULTIPLECHOICE_H
#define PARAMETERMULTIPLECHOICE_H

#include <string>
#include "config/configparameterbase.h"

class ParameterMultipleChoice : public ConfigParameterBase
{
public:

    /**
     * @brief Constructor for the ParameterMultipleChoice. This is a variant of the ConfigParameterBase in which the
     * parameter value is selected from among a preset list of options.
     * @param key
     *  The key used to identify the parameter in config file etc
     * @param title
     *  Descriptive name of the parameter
     * @param options
     *  The set of options
     * @param data
     *
     */
    ParameterMultipleChoice(const string key, const string title, const std::vector<string> options, string * const data);

    /**
     * @brief Pointer to the corresponding parameter in the state object, which is set whenever the parameter is
     * read from the GUI/config file and validated successfully
     */
    string * const data;

    /**
     * @brief The available options.
     */
    const std::vector<string> options;

    /**
     * @brief Validates the option selected by the user.
     */
    void parseAndValidate();

    /**
     * @brief Validates the option selected by the user.
     * @param option
     *  The option selected by the user.
     */
    void parseAndValidate(const string option);
};

#endif // PARAMETERMULTIPLECHOICE_H
