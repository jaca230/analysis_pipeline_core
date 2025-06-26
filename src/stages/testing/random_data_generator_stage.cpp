#include "stages/testing/random_data_generator_stage.h"

#include <TParameter.h>
#include <spdlog/spdlog.h>

ClassImp(RandomDataGeneratorStage)

RandomDataGeneratorStage::RandomDataGeneratorStage()
    : rng_(std::random_device{}()), dist_(0.0, 1.0) {}

void RandomDataGeneratorStage::OnInit() {
    productName_ = parameters_.value("product_name", "random_value");
    minValue_ = parameters_.value("min", 0.0);
    maxValue_ = parameters_.value("max", 1.0);
    seed_ = parameters_.value("seed", 0u);

    rng_.seed(seed_);
    dist_ = std::uniform_real_distribution<double>(minValue_, maxValue_);

    spdlog::debug("[{}] Initialized with name='{}', min={}, max={}, seed={}",
                 Name(), productName_, minValue_, maxValue_, seed_);
}

void RandomDataGeneratorStage::Process() {
    double randomValue = dist_(rng_);
    auto param = std::make_unique<TParameter<double>>(productName_.c_str(), randomValue);

    // Wrap in PipelineDataProduct and store
    auto product = std::make_unique<PipelineDataProduct>();
    product->setName(productName_);
    product->setObject(std::move(param));
    product->addTag("random");
    product->addTag("built_by_random_data_generator");

    // Overwrite product entry (thread-safe)
    getDataProductManager()->addOrUpdate(productName_, std::move(product));

    spdlog::debug("[{}] Generated value {} for '{}'", Name(), randomValue, productName_);
}



