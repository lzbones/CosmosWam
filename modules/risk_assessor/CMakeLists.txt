add_library(risk_assessor STATIC
    src/risk_assessor.cpp
)

target_include_directories(risk_assessor PUBLIC
    include
)
