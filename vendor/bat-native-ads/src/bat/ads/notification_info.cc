/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/notification_info.h"
#include "bat/ads/confirmation_type.h"

#include "bat/ads/internal/json_helper.h"

#include "base/logging.h"

namespace ads {

NotificationInfo::NotificationInfo() = default;

NotificationInfo::NotificationInfo(
    const NotificationInfo& info) = default;

NotificationInfo::~NotificationInfo() = default;

std::string NotificationInfo::ToJson() const {
  std::string json;
  SaveToJson(*this, &json);
  return json;
}

Result NotificationInfo::FromJson(
    const std::string& json,
    std::string* error_description) {
  rapidjson::Document document;
  document.Parse(json.c_str());

  if (document.HasParseError()) {
    if (error_description != nullptr) {
      *error_description = helper::JSON::GetLastError(&document);
    }

    return Result::kFailed;
  }

  if (document.HasMember("id")) {
    id = document["id"].GetString();
  }

  if (document.HasMember("creative_set_id")) {
    creative_set_id = document["creative_set_id"].GetString();
  }

  if (document.HasMember("category")) {
    category = document["category"].GetString();
  }

  if (document.HasMember("advertiser")) {
    advertiser = document["advertiser"].GetString();
  }

  if (document.HasMember("text")) {
    text = document["text"].GetString();
  }

  if (document.HasMember("url")) {
    url = document["url"].GetString();
  }

  if (document.HasMember("uuid")) {
    uuid = document["uuid"].GetString();
  }

  if (document.HasMember("confirmation_type")) {
    std::string confirmation_type = document["confirmation_type"].GetString();
    type = ConfirmationType(confirmation_type);
  }

  return Result::kSuccess;
}

void SaveToJson(
    JsonWriter* writer,
    const NotificationInfo& info) {
  writer->StartObject();

  writer->String("id");
  writer->String(info.id.c_str());

  writer->String("creative_set_id");
  writer->String(info.creative_set_id.c_str());

  writer->String("category");
  writer->String(info.category.c_str());

  writer->String("advertiser");
  writer->String(info.advertiser.c_str());

  writer->String("text");
  writer->String(info.text.c_str());

  writer->String("url");
  writer->String(info.url.c_str());

  writer->String("uuid");
  writer->String(info.uuid.c_str());

  writer->String("confirmation_type");
  auto type = std::string(info.type);
  writer->String(type.c_str());

  writer->EndObject();
}

}  // namespace ads
