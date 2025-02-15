#include "Distributor.h"

#include "Map.h"
#include "Utility.h"

void Distributor::Distribute(RE::TESObjectREFR* a_ref) noexcept
{
    const auto form_id{ a_ref->GetFormID() };
    const auto base_form_id{ a_ref->GetBaseObject()->GetFormID() };

    if (Map::processed_containers.contains(form_id)) {
        return;
    }

    const DistrVecs* to_modify{};

    if (Map::distr_map.contains(form_id)) {
        to_modify = &Map::distr_map[form_id];
    }
    else if (Map::distr_map.contains(base_form_id)) {
        to_modify = &Map::distr_map[base_form_id];
    }

    if (!to_modify) {
        return;
    }

    Map::processed_containers.insert(form_id);

    for (const auto& distr_obj : to_modify->to_add) {
        if (const auto& [type, container, bound_object, count, location, location_keyword, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            if (Utility::ShouldSkip(a_ref, location, location_keyword)) {
                continue;
            }
            if (const auto lev_item{ bound_object->As<RE::TESLevItem>() }) {
                Utility::AddObjectsFromResolvedList(a_ref, lev_item, count);
            }
            else {
                a_ref->AddObjectToContainer(bound_object, nullptr, count, nullptr);
                Map::added_objects[a_ref].emplace_back(bound_object, count);
                logger::info("+ {} / Container ref: {}", distr_obj, a_ref);
                logger::info("");
            }
        }
    }

    for (const auto& distr_obj : to_modify->to_remove) {
        if (const auto& [type, container, bound_object, count, location, location_keyword, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            if (bound_object->As<RE::TESLevItem>() || Utility::ShouldSkip(a_ref, location, location_keyword)) {
                continue;
            }
            a_ref->RemoveItem(bound_object, count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            logger::info("- {} / Container ref: {}", distr_obj, a_ref);
            logger::info("");
        }
    }

    for (const auto& distr_obj : to_modify->to_remove_all) {
        if (const auto& [type, container, bound_object, count, location, location_keyword, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            if (bound_object->As<RE::TESLevItem>() || Utility::ShouldSkip(a_ref, location, location_keyword)) {
                continue;
            }
            const auto inv_map{ a_ref->GetInventoryCounts() };
            if (!inv_map.contains(bound_object)) {
                logger::error("ERROR: Could not find {} in inventory counts map of {}", bound_object, a_ref);
                continue;
            }
            const auto inv_count{ inv_map.at(bound_object) };

            a_ref->RemoveItem(bound_object, inv_count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            logger::info("- {} / Remove all count: {} / Container ref: {}", distr_obj, inv_count, a_ref);
            logger::info("");
        }
    }
}
