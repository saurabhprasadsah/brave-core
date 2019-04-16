/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/sql_data_store_adapter.h"

#include <functional>

#include "base/no_destructor.h"
#include "bat/ledger/ledger_client.h"

using bat_ledger::mojom::DataStoreCommand;
using bat_ledger::mojom::DataStoreCommandPtr;
using bat_ledger::mojom::DataStoreCommandBinding;
// using bat_ledger::mojom::DataStoreCommandBindingPtr;
using bat_ledger::mojom::DataStoreCommandResponse;
// using bat_ledger::mojom::DataStoreRecord;
using bat_ledger::mojom::DataStoreRecordBinding;
// using bat_ledger::mojom::DataStoreRecordBindingPtr;
// using bat_ledger::mojom::DataStoreRecordPtr;
using bat_ledger::mojom::DataStoreTransaction;
using bat_ledger::mojom::DataStoreValue;
using std::placeholders::_1;

namespace ledger {

namespace {

const int kCurrentVersionNumber = 6;
const int kCompatibleVersionNumber = 1;

DataStoreCommandPtr CreateDataStoreCommand(int version) {
  auto command = DataStoreCommand::New();
  command->version = version;
  command->compatible_version = kCompatibleVersionNumber;
  return command;
}

using RecordFields = base::flat_map<std::string, DataStoreRecordBinding::Type>;

static const base::NoDestructor<RecordFields> publisher_info_fields({
    {"publisher_id", DataStoreRecordBinding::Type::INT_TYPE},
    {"verified", DataStoreRecordBinding::Type::BOOL_TYPE},
    {"excluded", DataStoreRecordBinding::Type::BOOL_TYPE},
    {"name", DataStoreRecordBinding::Type::STRING_TYPE},
    {"favIcon", DataStoreRecordBinding::Type::STRING_TYPE},
    {"url", DataStoreRecordBinding::Type::STRING_TYPE},
    {"provider", DataStoreRecordBinding::Type::STRING_TYPE},
});

static const base::NoDestructor<RecordFields> contribution_info_fields({
    {"publisher_id", DataStoreRecordBinding::Type::INT_TYPE},
    {"probi", DataStoreRecordBinding::Type::STRING_TYPE},
    {"date", DataStoreRecordBinding::Type::INT_TYPE},
    {"category", DataStoreRecordBinding::Type::INT_TYPE},
    {"month", DataStoreRecordBinding::Type::INT_TYPE},
    {"year", DataStoreRecordBinding::Type::INT_TYPE},
});

std::string GetSqlFields(
    const RecordFields& fields,
    const std::string& prefix = "") {
  std::string sql_fields = "";

  for (RecordFields::const_iterator it =
           fields.begin();
       it != fields.end(); ++it) {
    if (it != fields.begin()) {
      sql_fields += ",";
    }
    sql_fields += prefix + it->first;
  }

  return sql_fields;
}

}  // namespace

SqlDataStoreAdapter::SqlDataStoreAdapter() :
    testing_current_version_(-1) {}

// DataStoreTransactionPtr SqlDataStoreAdapter::Init() {
//   auto transaction = DataStoreTransaction::New();

//   transaction->commands.push_back(CreatePublisherInfoTable());
//   transaction->commands.push_back(CreateContributionInfoTable());
//   transaction->commands.push_back(CreateActivityInfoTable());
//   transaction->commands.push_back(CreateMediaPublisherInfoTable());
//   transaction->commands.push_back(CreateRecurringTipsTable());
//   transaction->commands.push_back(CreatePendingContributionsTable());
//   transaction->commands.push_back(CreateContributionInfoIndex());
//   transaction->commands.push_back(CreateActivityInfoIndex());
//   transaction->commands.push_back(CreateRecurringTipsIndex());
//   transaction->commands.push_back(CreatePendingContributionsIndex());
// }

// DataStoreCommandPtr SqlDataStoreAdapter::CreateContributionInfoTable() {
//   std::string sql;
//   sql.append("CREATE TABLE ");
//   sql.append("contribution_info");
//   sql.append(
//       "("
//       "publisher_id LONGVARCHAR,"
//       "probi TEXT \"0\"  NOT NULL,"
//       "date INTEGER NOT NULL,"
//       "category INTEGER NOT NULL,"
//       "month INTEGER NOT NULL,"
//       "year INTEGER NOT NULL,"
//       "CONSTRAINT fk_contribution_info_publisher_id"
//       "    FOREIGN KEY (publisher_id)"
//       "    REFERENCES publisher_info (publisher_id)"
//       "    ON DELETE CASCADE)");

//   auto command = CreateDataStoreCommand();
//   command->type = DataStoreCommand::Type::INITIALIZE;
//   command->command = sql;
//   return command;
// }

// DataStoreCommandPtr SqlDataStoreAdapter::CreateContributionInfoIndex() {
//   auto command = CreateDataStoreCommand();
//   command->type = DataStoreCommand::Type::INITIALIZE;
//   command->command =
//       "CREATE INDEX IF NOT EXISTS contribution_info_publisher_id_index "
//       "ON contribution_info (publisher_id)";
//   return command;
// }

int SqlDataStoreAdapter::GetCurrentVersion() {
  if (testing_current_version_ != -1) {
    return testing_current_version_;
  }

  return kCurrentVersionNumber;
}

void SqlDataStoreAdapter::SetCurrentVersionForTesting(int value) {
  testing_current_version_ = value;
}

void SqlDataStoreAdapter::Initialize(LedgerClient* ledger_client) {
  ledger_client_ = ledger_client;
}

void SqlDataStoreAdapter::SaveContributionInfo(
    const std::string& probi,
    const int month,
    const int year,
    const uint32_t date,
    const std::string& publisher_key,
    const ledger::REWARDS_CATEGORY category,
    SaveContributionInfoCallback callback) {
  std::string sql =
      "INSERT INTO contribution_info (" +
      GetSqlFields(*contribution_info_fields, "") +
      ") VALUES (?, ?, ?, ?, ?, ?)";

  auto command = CreateDataStoreCommand(GetCurrentVersion());
  command->type = DataStoreCommand::Type::CREATE;
  auto binding = DataStoreCommandBinding::New();
  binding->index = 0;
  binding->value->set_string_value(publisher_key);
  command->bindings.push_back(std::move(binding));

  binding = DataStoreCommandBinding::New();
  binding->index = 1;
  binding->value->set_string_value(probi);
  command->bindings.push_back(std::move(binding));

  binding = DataStoreCommandBinding::New();
  binding->index = 2;
  binding->value->set_int_value(date);
  command->bindings.push_back(std::move(binding));

  binding = DataStoreCommandBinding::New();
  binding->index = 3;
  binding->value->set_int_value(static_cast<int>(category));
  command->bindings.push_back(std::move(binding));

  binding = DataStoreCommandBinding::New();
  binding->index = 4;
  binding->value->set_int_value(month);
  command->bindings.push_back(std::move(binding));

  binding = DataStoreCommandBinding::New();
  binding->index = 5;
  binding->value->set_int_value(year);
  command->bindings.push_back(std::move(binding));

  auto transaction = DataStoreTransaction::New();
  transaction->commands.push_back(std::move(command));

  ledger_client_->RunDataStoreTransaction(std::move(transaction),
      std::bind(&SqlDataStoreAdapter::OnSaveContributionInfo,
                this,
                callback,
                _1));
}

void SqlDataStoreAdapter::OnSaveContributionInfo(
    SaveContributionInfoCallback callback,
    DataStoreCommandResponse* response) {
  ledger::Result result = ledger::Result::LEDGER_OK;
  if (response->status != DataStoreCommandResponse::Status::OK) {
    result = ledger::Result::LEDGER_ERROR;
  }

  callback(result);
}

void SqlDataStoreAdapter::GetRecurringTips(PublisherInfoListCallback callback) {
  std::string sql =
      "SELECT pi.publisher_id, pi.verified, pi.name, pi.url, pi.favIcon, "
      "rd.amount, rd.added_date, , pi.provider "
      "FROM recurring_donation as rd "
      "INNER JOIN publisher_info AS pi ON rd.publisher_id = pi.publisher_id";
  auto command = CreateDataStoreCommand(GetCurrentVersion());
  command->type = DataStoreCommand::Type::READ;

  auto transaction = DataStoreTransaction::New();
  transaction->commands.push_back(std::move(command));

  ledger_client_->RunDataStoreTransaction(std::move(transaction),
      std::bind(&SqlDataStoreAdapter::OnGetPublisherInfo,
                this,
                callback,
                _1));

//   sql::Statement info_sql(db_.GetUniqueStatement(
// -      "SELECT pi.publisher_id, pi.name, pi.url, pi.favIcon, "
// -      "rd.amount, rd.added_date, pi.verified, pi.provider "
// -      "FROM recurring_donation as rd "
// -      "INNER JOIN publisher_info AS pi ON rd.publisher_id = pi.publisher_id "));
// -
// -  while (info_sql.Step()) {
// -    std::string id(info_sql.ColumnString(0));
// -
// -    ledger::PublisherInfo publisher(id);
// -    publisher.name = info_sql.ColumnString(1);
// -    publisher.url = info_sql.ColumnString(2);
// -    publisher.favicon_url = info_sql.ColumnString(3);
// -    publisher.weight = info_sql.ColumnDouble(4);
// -    publisher.reconcile_stamp = info_sql.ColumnInt64(5);
// -    publisher.verified = info_sql.ColumnBool(6);
// -    publisher.provider = info_sql.ColumnString(7);
}

void SqlDataStoreAdapter::OnGetPublisherInfo(
    PublisherInfoListCallback callback,
    DataStoreCommandResponse* response) {
  PublisherInfoList list;
  if (response->status != DataStoreCommandResponse::Status::OK) {
    callback(list, 0);
    return;
  }

  // for (auto const& record : response->result->get_records()) {

    // ledger::PublisherInfo publisher()
  // }
  //   while (info_sql.Step()) {
//     std::string id(info_sql.ColumnString(0));

//     ledger::PublisherInfo publisher(id);

//     publisher.name = info_sql.ColumnString(1);
//     publisher.url = info_sql.ColumnString(2);
//     publisher.favicon_url = info_sql.ColumnString(3);
//     publisher.weight = info_sql.ColumnDouble(4);
//     publisher.reconcile_stamp = info_sql.ColumnInt64(5);
//     publisher.verified = info_sql.ColumnBool(6);
//     publisher.provider = info_sql.ColumnString(7);

//     list->push_back(publisher);
//   }
// }
  // TODO - convert to PublisherInfoList
  // callback(result);
}

void SqlDataStoreAdapter::GetOneTimeTips(
    ledger::ACTIVITY_MONTH month,
    int year,
    ledger::PublisherInfoListCallback callback) {
  std::string sql =
      "SELECT " + GetSqlFields(*publisher_info_fields, "pi.") + "," +
      GetSqlFields(*contribution_info_fields, "ci.") + " " +
      "FROM contribution_info as ci "
      "INNER JOIN publisher_info AS pi ON ci.publisher_id = pi.publisher_id "
      "AND ci.month = ? AND ci.year = ? "
      "AND ci.category = ?";

  auto command = CreateDataStoreCommand(GetCurrentVersion());
  command->type = DataStoreCommand::Type::READ;

  auto binding = DataStoreCommandBinding::New();
  binding->index = 0;
  binding->value->set_int_value(month);
  command->bindings.push_back(std::move(binding));

  binding = DataStoreCommandBinding::New();
  binding->index = 1;
  binding->value->set_int_value(year);
  command->bindings.push_back(std::move(binding));

  binding = DataStoreCommandBinding::New();
  binding->index = 2;
  binding->value->set_int_value(ledger::REWARDS_CATEGORY::ONE_TIME_TIP);
  command->bindings.push_back(std::move(binding));

  auto transaction = DataStoreTransaction::New();
  transaction->commands.push_back(std::move(command));

  ledger_client_->RunDataStoreTransaction(std::move(transaction),
      std::bind(&SqlDataStoreAdapter::OnGetPublisherInfo,
                this,
                callback,
                _1));
}

// DataStoreCommandPtr SqlDataStoreAdapter::CreatePublisherInfoTable() {
//   std::string sql;
//   sql.append("CREATE TABLE publisher_info");
//   sql.append(
//       "("
//       "publisher_id LONGVARCHAR PRIMARY KEY NOT NULL UNIQUE,"
//       "verified BOOLEAN DEFAULT 0 NOT NULL,"
//       "excluded INTEGER DEFAULT 0 NOT NULL,"
//       "name TEXT NOT NULL,"
//       "favIcon TEXT NOT NULL,"
//       "url TEXT NOT NULL,"
//       "provider TEXT NOT NULL)");

//   auto command = DataStoreCommand::New();
//   command->type = DataStoreCommand::Type::EXECUTE;
//   command->command = sql;
//   command->version = kCurrentVersionNumber;
//   command->compatible_version = kCompatibleVersionNumber;
//   return command;
// }

// bool SqlDataStoreAdapter::InsertOrUpdatePublisherInfo(
//     const ledger::PublisherInfo& info) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized || info.id.empty()) {
//     return false;
//   }

//   sql::Transaction transaction(&GetDB());
//   if (!transaction.Begin()) {
//     return false;
//   }

//   sql::Statement publisher_info_statement(
//       GetDB().GetCachedStatement(SQL_FROM_HERE,
//                                  "INSERT OR REPLACE INTO publisher_info "
//                                  "(publisher_id, verified, excluded, "
//                                  "name, url, provider, favIcon) "
//                                  "VALUES (?, ?, ?, ?, ?, ?, "
//                                  "(SELECT IFNULL( "
//                                  "(SELECT favicon FROM publisher_info "
//                                  "WHERE publisher_id = ?), \"\"))"
//                                  ")"));

//   publisher_info_statement.BindString(0, info.id);
//   publisher_info_statement.BindBool(1, info.verified);
//   publisher_info_statement.BindInt(2, static_cast<int>(info.excluded));
//   publisher_info_statement.BindString(3, info.name);
//   publisher_info_statement.BindString(4, info.url);
//   publisher_info_statement.BindString(5, info.provider);
//   publisher_info_statement.BindString(6, info.id);

//   publisher_info_statement.Run();

//   std::string favicon = info.favicon_url;
//   if (!favicon.empty()) {
//     sql::Statement favicon_statement(
//       GetDB().GetCachedStatement(SQL_FROM_HERE,
//                                  "UPDATE publisher_info SET favIcon = ? "
//                                  "WHERE publisher_id = ?"));

//     if (favicon == ledger::_clear_favicon) {
//       favicon.clear();
//     }

//     favicon_statement.BindString(0, favicon);
//     favicon_statement.BindString(1, info.id);

//     favicon_statement.Run();
//   }

//   return transaction.Commit();
// }

// std::unique_ptr<ledger::PublisherInfo>
// SqlDataStoreAdapter::GetPublisherInfo(const std::string& publisher_key) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized) {
//     return nullptr;
//   }

//   sql::Statement info_sql(db_.GetUniqueStatement(
//       "SELECT publisher_id, name, url, favIcon, provider, verified, excluded "
//       "FROM publisher_info WHERE publisher_id=?"));

//   info_sql.BindString(0, publisher_key);

//   if (info_sql.Step()) {
//     std::unique_ptr<ledger::PublisherInfo> info;
//     info.reset(new ledger::PublisherInfo());
//     info->id = info_sql.ColumnString(0);
//     info->name = info_sql.ColumnString(1);
//     info->url = info_sql.ColumnString(2);
//     info->favicon_url = info_sql.ColumnString(3);
//     info->provider = info_sql.ColumnString(4);
//     info->verified = info_sql.ColumnBool(5);
//     info->excluded = static_cast<ledger::PUBLISHER_EXCLUDE>(
//         info_sql.ColumnInt(6));

//     return info;
//   }

//   return nullptr;
// }

// std::unique_ptr<ledger::PublisherInfo>
// SqlDataStoreAdapter::GetPanelPublisher(
//     const ledger::ActivityInfoFilter& filter) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized || filter.id.empty()) {
//     return nullptr;
//   }

//   sql::Statement info_sql(db_.GetUniqueStatement(
//       "SELECT pi.publisher_id, pi.name, pi.url, pi.favIcon, "
//       "pi.provider, pi.verified, pi.excluded, "
//       "("
//       "SELECT IFNULL(percent, 0) FROM activity_info WHERE "
//       "publisher_id = ? AND reconcile_stamp = ? "
//       ") as percent "
//       "FROM publisher_info AS pi WHERE pi.publisher_id = ? LIMIT 1"));

//   info_sql.BindString(0, filter.id);
//   info_sql.BindInt64(1, filter.reconcile_stamp);
//   info_sql.BindString(2, filter.id);

//   if (info_sql.Step()) {
//     std::unique_ptr<ledger::PublisherInfo> info;
//     info.reset(new ledger::PublisherInfo());
//     info->id = info_sql.ColumnString(0);
//     info->name = info_sql.ColumnString(1);
//     info->url = info_sql.ColumnString(2);
//     info->favicon_url = info_sql.ColumnString(3);
//     info->provider = info_sql.ColumnString(4);
//     info->verified = info_sql.ColumnBool(5);
//     info->excluded = static_cast<ledger::PUBLISHER_EXCLUDE>(
//         info_sql.ColumnInt(6));
//     info->percent = info_sql.ColumnInt(7);

//     return info;
//   }

//   return nullptr;
// }

// bool SqlDataStoreAdapter::RestorePublishers() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized) {
//     return false;
//   }

//   sql::Statement restore_q(db_.GetUniqueStatement(
//       "UPDATE publisher_info SET excluded=? WHERE excluded=?"));

//   restore_q.BindInt(0, static_cast<int>(
//       ledger::PUBLISHER_EXCLUDE::DEFAULT));
//   restore_q.BindInt(1, static_cast<int>(
//       ledger::PUBLISHER_EXCLUDE::EXCLUDED));

//   return restore_q.Run();
// }

// int SqlDataStoreAdapter::GetExcludedPublishersCount() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized) {
//     return 0;
//   }

//   sql::Statement query(db_.GetUniqueStatement(
//       "SELECT COUNT(*) FROM publisher_info WHERE excluded=?"));

//   query.BindInt(0, static_cast<int>(
//       ledger::PUBLISHER_EXCLUDE::EXCLUDED));

//   if (query.Step()) {
//     return query.ColumnInt(0);
//   }

//   return 0;
// }

// /**
//  *
//  * ACTIVITY INFO
//  *
//  */
// bool SqlDataStoreAdapter::CreateActivityInfoTable() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   const char* name = "activity_info";
//   if (GetDB().DoesTableExist(name)) {
//     return true;
//   }

//   std::string sql;
//   sql.append("CREATE TABLE ");
//   sql.append(name);
//   sql.append(
//       "("
//       "publisher_id LONGVARCHAR NOT NULL,"
//       "duration INTEGER DEFAULT 0 NOT NULL,"
//       "visits INTEGER DEFAULT 0 NOT NULL,"
//       "score DOUBLE DEFAULT 0 NOT NULL,"
//       "percent INTEGER DEFAULT 0 NOT NULL,"
//       "weight DOUBLE DEFAULT 0 NOT NULL,"
//       "reconcile_stamp INTEGER DEFAULT 0 NOT NULL,"
//       "CONSTRAINT activity_unique "
//       "UNIQUE (publisher_id, reconcile_stamp) "
//       "CONSTRAINT fk_activity_info_publisher_id"
//       "    FOREIGN KEY (publisher_id)"
//       "    REFERENCES publisher_info (publisher_id)"
//       "    ON DELETE CASCADE)");

//   return GetDB().Execute(sql.c_str());
// }

// bool SqlDataStoreAdapter::CreateActivityInfoIndex() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   return GetDB().Execute(
//       "CREATE INDEX IF NOT EXISTS activity_info_publisher_id_index "
//       "ON activity_info (publisher_id)");
// }

// bool SqlDataStoreAdapter::InsertOrUpdateActivityInfo(
//     const ledger::PublisherInfo& info) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized || info.id.empty()) {
//     return false;
//   }

//   if (!InsertOrUpdatePublisherInfo(info)) {
//     return false;
//   }

//   sql::Statement activity_info_insert(
//     GetDB().GetCachedStatement(SQL_FROM_HERE,
//         "INSERT OR REPLACE INTO activity_info "
//         "(publisher_id, duration, score, percent, "
//         "weight, reconcile_stamp, visits) "
//         "VALUES (?, ?, ?, ?, ?, ?, ?)"));

//   activity_info_insert.BindString(0, info.id);
//   activity_info_insert.BindInt64(1, static_cast<int>(info.duration));
//   activity_info_insert.BindDouble(2, info.score);
//   activity_info_insert.BindInt64(3, static_cast<int>(info.percent));
//   activity_info_insert.BindDouble(4, info.weight);
//   activity_info_insert.BindInt64(5, info.reconcile_stamp);
//   activity_info_insert.BindInt(6, info.visits);

//   return activity_info_insert.Run();
// }

// bool SqlDataStoreAdapter::InsertOrUpdateActivityInfos(
//     const ledger::PublisherInfoList& list) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized || list.size() == 0) {
//     return false;
//   }

//   sql::Transaction transaction(&GetDB());
//   if (!transaction.Begin()) {
//     return false;
//   }

//   for (const auto& info : list) {
//     if (!InsertOrUpdateActivityInfo(info)) {
//       transaction.Rollback();
//       return false;
//     }
//   }

//   return transaction.Commit();
// }

// bool SqlDataStoreAdapter::GetActivityList(
//     int start,
//     int limit,
//     const ledger::ActivityInfoFilter& filter,
//     ledger::PublisherInfoList* list) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   CHECK(list);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized) {
//     return false;
//   }

//   std::string query = "SELECT ai.publisher_id, ai.duration, ai.score, "
//                       "ai.percent, ai.weight, pi.verified, pi.excluded, "
//                       "pi.name, pi.url, pi.provider, "
//                       "pi.favIcon, ai.reconcile_stamp, ai.visits "
//                       "FROM activity_info AS ai "
//                       "INNER JOIN publisher_info AS pi "
//                       "ON ai.publisher_id = pi.publisher_id "
//                       "WHERE 1 = 1";

//   if (!filter.id.empty()) {
//     query += " AND ai.publisher_id = ?";
//   }

//   if (filter.reconcile_stamp > 0) {
//     query += " AND ai.reconcile_stamp = ?";
//   }

//   if (filter.min_duration > 0) {
//     query += " AND ai.duration >= ?";
//   }

//   if (filter.excluded != ledger::EXCLUDE_FILTER::FILTER_ALL &&
//       filter.excluded !=
//         ledger::EXCLUDE_FILTER::FILTER_ALL_EXCEPT_EXCLUDED) {
//     query += " AND pi.excluded = ?";
//   }

//   if (filter.excluded ==
//     ledger::EXCLUDE_FILTER::FILTER_ALL_EXCEPT_EXCLUDED) {
//     query += " AND pi.excluded != ?";
//   }

//   if (filter.percent > 0) {
//     query += " AND ai.percent >= ?";
//   }

//   if (filter.min_visits > 0) {
//     query += " AND ai.visits >= ?";
//   }

//   if (!filter.non_verified) {
//     query += " AND pi.verified = 1";
//   }

//   for (const auto& it : filter.order_by) {
//     query += " ORDER BY " + it.first;
//     query += (it.second ? " ASC" : " DESC");
//   }

//   if (limit > 0) {
//     query += " LIMIT " + std::to_string(limit);

//     if (start > 1) {
//       query += " OFFSET " + std::to_string(start);
//     }
//   }

//   sql::Statement info_sql(db_.GetUniqueStatement(query.c_str()));

//   int column = 0;
//   if (!filter.id.empty()) {
//     info_sql.BindString(column++, filter.id);
//   }

//   if (filter.reconcile_stamp > 0) {
//     info_sql.BindInt64(column++, filter.reconcile_stamp);
//   }

//   if (filter.min_duration > 0) {
//     info_sql.BindInt(column++, filter.min_duration);
//   }

//   if (filter.excluded != ledger::EXCLUDE_FILTER::FILTER_ALL &&
//       filter.excluded !=
//       ledger::EXCLUDE_FILTER::FILTER_ALL_EXCEPT_EXCLUDED) {
//     info_sql.BindInt(column++, filter.excluded);
//   }

//   if (filter.excluded ==
//       ledger::EXCLUDE_FILTER::FILTER_ALL_EXCEPT_EXCLUDED) {
//     info_sql.BindInt(column++, ledger::PUBLISHER_EXCLUDE::EXCLUDED);
//   }

//   if (filter.percent > 0) {
//     info_sql.BindInt(column++, filter.percent);
//   }

//   if (filter.min_visits > 0) {
//     info_sql.BindInt(column++, filter.min_visits);
//   }

//   while (info_sql.Step()) {
//     std::string id(info_sql.ColumnString(0));

//     ledger::PublisherInfo info(id);
//     info.duration = info_sql.ColumnInt64(1);
//     info.score = info_sql.ColumnDouble(2);
//     info.percent = info_sql.ColumnInt64(3);
//     info.weight = info_sql.ColumnDouble(4);
//     info.verified = info_sql.ColumnBool(5);
//     info.excluded = static_cast<ledger::PUBLISHER_EXCLUDE>(
//         info_sql.ColumnInt(6));
//     info.name = info_sql.ColumnString(7);
//     info.url = info_sql.ColumnString(8);
//     info.provider = info_sql.ColumnString(9);
//     info.favicon_url = info_sql.ColumnString(10);
//     info.reconcile_stamp = info_sql.ColumnInt64(11);
//     info.visits = info_sql.ColumnInt(12);

//     list->push_back(info);
//   }

//   return true;
// }

// bool SqlDataStoreAdapter::DeleteActivityInfo(
//     const std::string& publisher_key,
//     uint64_t reconcile_stamp) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized || publisher_key.empty() || reconcile_stamp == 0) {
//     return false;
//   }

//   sql::Statement statement(GetDB().GetCachedStatement(
//       SQL_FROM_HERE,
//       "DELETE FROM activity_info WHERE "
//       "publisher_id = ? AND reconcile_stamp = ?"));

//   statement.BindString(0, publisher_key);
//   statement.BindInt64(1, reconcile_stamp);

//   return statement.Run();
// }

// /**
//  *
//  * MEDIA PUBLISHER INFO
//  *
//  */
// bool SqlDataStoreAdapter::CreateMediaPublisherInfoTable() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   const char* name = "media_publisher_info";
//   if (GetDB().DoesTableExist(name)) {
//     return true;
//   }

//   std::string sql;
//   sql.append("CREATE TABLE ");
//   sql.append(name);
//   sql.append(
//       "("
//       "media_key TEXT NOT NULL PRIMARY KEY UNIQUE,"
//       "publisher_id LONGVARCHAR NOT NULL,"
//       "CONSTRAINT fk_media_publisher_info_publisher_id"
//       "    FOREIGN KEY (publisher_id)"
//       "    REFERENCES publisher_info (publisher_id)"
//       "    ON DELETE CASCADE)");

//   return GetDB().Execute(sql.c_str());
// }

// bool SqlDataStoreAdapter::InsertOrUpdateMediaPublisherInfo(
//     const std::string& media_key, const std::string& publisher_id) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized || media_key.empty() || publisher_id.empty()) {
//     return false;
//   }

//   sql::Statement statement(GetDB().GetCachedStatement(
//       SQL_FROM_HERE,
//       "INSERT OR REPLACE INTO media_publisher_info "
//       "(media_key, publisher_id) "
//       "VALUES (?, ?)"));

//   statement.BindString(0, media_key);
//   statement.BindString(1, publisher_id);

//   return statement.Run();
// }

// std::unique_ptr<ledger::PublisherInfo>
// SqlDataStoreAdapter::GetMediaPublisherInfo(const std::string& media_key) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized) {
//     return nullptr;
//   }

//   sql::Statement info_sql(db_.GetUniqueStatement(
//       "SELECT pi.publisher_id, pi.name, pi.url, pi.favIcon, "
//       "pi.provider, pi.verified, pi.excluded "
//       "FROM media_publisher_info as mpi "
//       "INNER JOIN publisher_info AS pi ON mpi.publisher_id = pi.publisher_id "
//       "WHERE mpi.media_key=?"));

//   info_sql.BindString(0, media_key);

//   if (info_sql.Step()) {
//     std::unique_ptr<ledger::PublisherInfo> info(new ledger::PublisherInfo());
//     info->id = info_sql.ColumnString(0);
//     info->name = info_sql.ColumnString(1);
//     info->url = info_sql.ColumnString(2);
//     info->favicon_url = info_sql.ColumnString(3);
//     info->provider = info_sql.ColumnString(4);
//     info->verified = info_sql.ColumnBool(5);
//     info->excluded = static_cast<ledger::PUBLISHER_EXCLUDE>(
//         info_sql.ColumnInt(6));

//     return info;
//   }

//   return nullptr;
// }

// /**
//  *
//  * RECURRING TIPS
//  *
//  */
// bool SqlDataStoreAdapter::CreateRecurringTipsTable() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   // TODO(nejczdovc): migrate name of this table from donation to tips
//   const char* name = "recurring_donation";
//   if (GetDB().DoesTableExist(name)) {
//     return true;
//   }

//   std::string sql;
//   sql.append("CREATE TABLE ");
//   sql.append(name);
//   sql.append(
//       "("
//       "publisher_id LONGVARCHAR NOT NULL PRIMARY KEY UNIQUE,"
//       "amount DOUBLE DEFAULT 0 NOT NULL,"
//       "added_date INTEGER DEFAULT 0 NOT NULL,"
//       "CONSTRAINT fk_recurring_donation_publisher_id"
//       "    FOREIGN KEY (publisher_id)"
//       "    REFERENCES publisher_info (publisher_id)"
//       "    ON DELETE CASCADE)");

//   return GetDB().Execute(sql.c_str());
// }

// bool SqlDataStoreAdapter::CreateRecurringTipsIndex() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   return GetDB().Execute(
//       "CREATE INDEX IF NOT EXISTS recurring_donation_publisher_id_index "
//       "ON recurring_donation (publisher_id)");
// }

// bool SqlDataStoreAdapter::InsertOrUpdateRecurringTip(
//     const brave_rewards::RecurringDonation& info) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized || info.publisher_key.empty()) {
//     return false;
//   }

//   sql::Statement statement(GetDB().GetCachedStatement(
//       SQL_FROM_HERE,
//       "INSERT OR REPLACE INTO recurring_donation "
//       "(publisher_id, amount, added_date) "
//       "VALUES (?, ?, ?)"));

//   statement.BindString(0, info.publisher_key);
//   statement.BindDouble(1, info.amount);
//   statement.BindInt64(2, info.added_date);

//   return statement.Run();
// }

// void SqlDataStoreAdapter::GetRecurringTips(
//     ledger::PublisherInfoList* list) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized) {
//     return;
//   }

//   sql::Statement info_sql(db_.GetUniqueStatement(
//       "SELECT pi.publisher_id, pi.name, pi.url, pi.favIcon, "
//       "rd.amount, rd.added_date, pi.verified, pi.provider "
//       "FROM recurring_donation as rd "
//       "INNER JOIN publisher_info AS pi ON rd.publisher_id = pi.publisher_id "));

//   while (info_sql.Step()) {
//     std::string id(info_sql.ColumnString(0));

//     ledger::PublisherInfo publisher(id);
//     publisher.name = info_sql.ColumnString(1);
//     publisher.url = info_sql.ColumnString(2);
//     publisher.favicon_url = info_sql.ColumnString(3);
//     publisher.weight = info_sql.ColumnDouble(4);
//     publisher.reconcile_stamp = info_sql.ColumnInt64(5);
//     publisher.verified = info_sql.ColumnBool(6);
//     publisher.provider = info_sql.ColumnString(7);

//     list->push_back(publisher);
//   }
// }

// bool SqlDataStoreAdapter::RemoveRecurringTip(
//     const std::string& publisher_key) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized) {
//     return false;
//   }

//   sql::Statement statement(GetDB().GetCachedStatement(
//       SQL_FROM_HERE,
//       "DELETE FROM recurring_donation WHERE publisher_id = ?"));

//   statement.BindString(0, publisher_key);

//   return statement.Run();
// }

// /**
//  *
//  * PENDING CONTRIBUTION
//  *
//  */
// bool SqlDataStoreAdapter::CreatePendingContributionsTable() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   const char* name = "pending_contribution";
//   if (GetDB().DoesTableExist(name)) {
//     return true;
//   }

//   std::string sql;
//   sql.append("CREATE TABLE ");
//   sql.append(name);
//   sql.append(
//       "("
//       "publisher_id LONGVARCHAR NOT NULL,"
//       "amount DOUBLE DEFAULT 0 NOT NULL,"
//       "added_date INTEGER DEFAULT 0 NOT NULL,"
//       "viewing_id LONGVARCHAR NOT NULL,"
//       "category INTEGER NOT NULL,"
//       "CONSTRAINT fk_pending_contribution_publisher_id"
//       "    FOREIGN KEY (publisher_id)"
//       "    REFERENCES publisher_info (publisher_id)"
//       "    ON DELETE CASCADE)");
//   return GetDB().Execute(sql.c_str());
// }

// bool SqlDataStoreAdapter::CreatePendingContributionsIndex() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   return GetDB().Execute(
//       "CREATE INDEX IF NOT EXISTS pending_contribution_publisher_id_index "
//       "ON pending_contribution (publisher_id)");
// }

// bool SqlDataStoreAdapter::InsertPendingContribution
// (const ledger::PendingContributionList& list) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   if (!initialized) {
//     return false;
//   }

//   base::Time now = base::Time::Now();
//   double now_seconds = now.ToDoubleT();

//   sql::Transaction transaction(&GetDB());
//   if (!transaction.Begin()) {
//     return false;
//   }

//   for (const auto& item : list.list_) {
//     sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
//       "INSERT INTO pending_contribution "
//       "(publisher_id, amount, added_date, viewing_id, category) "
//       "VALUES (?, ?, ?, ?, ?)"));

//     statement.BindString(0, item.publisher_key);
//     statement.BindDouble(1, item.amount);
//     statement.BindInt64(2, now_seconds);
//     statement.BindString(3, item.viewing_id);
//     statement.BindInt(4, item.category);
//     statement.Run();
//   }

//   return transaction.Commit();
// }

// double SqlDataStoreAdapter::GetReservedAmount() {
//     DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   bool initialized = Init();
//   DCHECK(initialized);

//   double amount = 0.0;

//   if (!initialized) {
//     return amount;
//   }

//   sql::Statement info_sql(
//       db_.GetUniqueStatement("SELECT sum(amount) FROM pending_contribution"));

//   if (info_sql.Step()) {
//     amount = info_sql.ColumnDouble(0);
//   }

//   return amount;
// }

// int SqlDataStoreAdapter::GetCurrentVersion() {
//   if (testing_current_version_ != -1) {
//     return testing_current_version_;
//   }

//   return kCurrentVersionNumber;
// }

// void SqlDataStoreAdapter::SetTestingCurrentVersion(int value) {
//   testing_current_version_ = value;
// }

// void SqlDataStoreAdapter::Vacuum() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   if (!initialized_)
//     return;

//   DCHECK_EQ(0, db_.transaction_nesting()) <<
//       "Can not have a transaction when vacuuming.";
//   ignore_result(db_.Execute("VACUUM"));
// }

// void SqlDataStoreAdapter::OnMemoryPressure(
//     base::MemoryPressureListener::MemoryPressureLevel memory_pressure_level) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
//   db_.TrimMemory();
// }

// std::string SqlDataStoreAdapter::GetDiagnosticInfo(int extended_error,
//                                                sql::Statement* statement) {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   DCHECK(initialized_);
//   return db_.GetDiagnosticInfo(extended_error, statement);
// }

// sql::Database& SqlDataStoreAdapter::GetDB() {
//   return db_;
// }

// sql::MetaTable& SqlDataStoreAdapter::GetMetaTable() {
//   return meta_table_;
// }

// int SqlDataStoreAdapter::GetTableVersionNumber() {
//   return meta_table_.GetVersionNumber();
// }

// // Migration -------------------------------------------------------------------

// bool SqlDataStoreAdapter::MigrateV1toV2() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   std::string sql;

//   // Activity info
//   const char* activity = "activity_info";
//   if (GetDB().DoesTableExist(activity)) {
//     const char* column = "reconcile_stamp";
//     if (!GetDB().DoesColumnExist(activity, column)) {
//       sql.append(" ALTER TABLE ");
//       sql.append(activity);
//       sql.append(" ADD reconcile_stamp INTEGER DEFAULT 0 NOT NULL; ");
//     }
//   }

//   // Contribution info
//   const char* contribution = "contribution_info";
//   if (GetDB().DoesTableExist(contribution)) {
//     sql.append(" DROP TABLE ");
//     sql.append(contribution);
//     sql.append(" ; ");
//   }

//   if (!GetDB().Execute(sql.c_str())) {
//     return false;
//   }

//   const char* name = "contribution_info";
//   sql = "CREATE TABLE ";
//   sql.append(name);
//   sql.append(
//       "("
//       "publisher_id LONGVARCHAR,"
//       "probi TEXT \"0\"  NOT NULL,"
//       "date INTEGER NOT NULL,"
//       "category INTEGER NOT NULL,"
//       "month INTEGER NOT NULL,"
//       "year INTEGER NOT NULL,"
//       "CONSTRAINT fk_contribution_info_publisher_id"
//       "    FOREIGN KEY (publisher_id)"
//       "    REFERENCES publisher_info (publisher_id)"
//       "    ON DELETE CASCADE)");
//   if (!GetDB().Execute(sql.c_str())) {
//     return false;
//   }

//   if (!GetDB().Execute(
//       "CREATE INDEX IF NOT EXISTS contribution_info_publisher_id_index "
//       "ON contribution_info (publisher_id)")) {
//     return false;
//   }

//   // Recurring_donation
//   name = "recurring_donation";
//   if (GetDB().DoesTableExist(name)) {
//     sql = " DROP TABLE ";
//     sql.append(name);
//     sql.append(" ; ");
//   }

//   if (!GetDB().Execute(sql.c_str())) {
//     return false;
//   }

//   sql = "CREATE TABLE ";
//   sql.append(name);
//   sql.append(
//       "("
//       "publisher_id LONGVARCHAR NOT NULL PRIMARY KEY UNIQUE,"
//       "amount DOUBLE DEFAULT 0 NOT NULL,"
//       "added_date INTEGER DEFAULT 0 NOT NULL,"
//       "CONSTRAINT fk_recurring_donation_publisher_id"
//       "    FOREIGN KEY (publisher_id)"
//       "    REFERENCES publisher_info (publisher_id)"
//       "    ON DELETE CASCADE)");
//   if (!GetDB().Execute(sql.c_str())) {
//     return false;
//   }

//   return GetDB().Execute(
//       "CREATE INDEX IF NOT EXISTS recurring_donation_publisher_id_index "
//       "ON recurring_donation (publisher_id)");
// }

// bool SqlDataStoreAdapter::MigrateV2toV3() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   std::string sql;
//   const char* name = "pending_contribution";
//   // pending_contribution
//   const char* pending_contribution = "pending_contribution";
//   if (GetDB().DoesTableExist(pending_contribution)) {
//     sql.append(" DROP TABLE ");
//     sql.append(pending_contribution);
//     sql.append(" ; ");
//   }
//   if (!GetDB().Execute(sql.c_str())) {
//     return false;
//   }

//   sql = "CREATE TABLE ";
//   sql.append(name);
//   sql.append(
//       "("
//       "publisher_id LONGVARCHAR NOT NULL,"
//       "amount DOUBLE DEFAULT 0 NOT NULL,"
//       "added_date INTEGER DEFAULT 0 NOT NULL,"
//       "viewing_id LONGVARCHAR NOT NULL,"
//       "category INTEGER NOT NULL,"
//       "CONSTRAINT fk_pending_contribution_publisher_id"
//       "    FOREIGN KEY (publisher_id)"
//       "    REFERENCES publisher_info (publisher_id)"
//       "    ON DELETE CASCADE)");
//   if (!GetDB().Execute(sql.c_str())) {
//     return false;
//   }

//   return GetDB().Execute(
//       "CREATE INDEX IF NOT EXISTS pending_contribution_publisher_id_index "
//       "ON pending_contribution (publisher_id)");
// }

// bool SqlDataStoreAdapter::MigrateV3toV4() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   // Activity info
//   const char* name = "activity_info";
//   if (GetDB().DoesTableExist(name)) {
//     std::string sql = "ALTER TABLE activity_info RENAME TO activity_info_old;";

//     if (!GetDB().Execute(sql.c_str())) {
//       return false;
//     }

//     sql = "CREATE TABLE ";
//     sql.append(name);
//     sql.append(
//         "("
//         "publisher_id LONGVARCHAR NOT NULL,"
//         "duration INTEGER DEFAULT 0 NOT NULL,"
//         "visits INTEGER DEFAULT 0 NOT NULL,"
//         "score DOUBLE DEFAULT 0 NOT NULL,"
//         "percent INTEGER DEFAULT 0 NOT NULL,"
//         "weight DOUBLE DEFAULT 0 NOT NULL,"
//         "month INTEGER NOT NULL,"
//         "year INTEGER NOT NULL,"
//         "reconcile_stamp INTEGER DEFAULT 0 NOT NULL,"
//         "CONSTRAINT activity_unique "
//         "UNIQUE (publisher_id, month, year, reconcile_stamp) "
//         "CONSTRAINT fk_activity_info_publisher_id"
//         "    FOREIGN KEY (publisher_id)"
//         "    REFERENCES publisher_info (publisher_id)"
//         "    ON DELETE CASCADE)");
//     if (!GetDB().Execute(sql.c_str())) {
//       return false;
//     }

//     if (!GetDB().Execute(
//       "CREATE INDEX IF NOT EXISTS activity_info_publisher_id_index "
//       "ON activity_info (publisher_id)")) {
//       return false;
//     }

//     std::string columns = "publisher_id, "
//                           "duration, "
//                           "score, "
//                           "percent, "
//                           "weight, "
//                           "month, "
//                           "year, "
//                           "reconcile_stamp";

//     sql = "PRAGMA foreign_keys=off;";
//     sql.append("INSERT INTO activity_info (" + columns + ") "
//                "SELECT " + columns + " "
//                "FROM activity_info_old;");
//     sql.append("UPDATE activity_info SET visits=5;");
//     sql.append("DROP TABLE activity_info_old;");
//     sql.append("PRAGMA foreign_keys=on;");

//     return GetDB().Execute(sql.c_str());
//   }

//   return false;
// }

// bool SqlDataStoreAdapter::MigrateV4toV5() {
//   sql::Transaction transaction(&GetDB());
//   if (!transaction.Begin()) {
//     return false;
//   }

//   sql::Statement info_sql(db_.GetUniqueStatement(
//       "SELECT publisher_id, month, year, reconcile_stamp "
//       "FROM activity_info "
//       "WHERE visits = 0"));

//   while (info_sql.Step()) {
//     sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
//       "UPDATE activity_info SET visits = 1 "
//       "WHERE publisher_id = ? AND month = ? AND "
//       "year = ? AND reconcile_stamp = ?"));

//     statement.BindString(0, info_sql.ColumnString(0));
//     statement.BindInt(1, info_sql.ColumnInt(1));
//     statement.BindInt(2, info_sql.ColumnInt(2));
//     statement.BindInt64(3, info_sql.ColumnInt64(3));
//     statement.Run();
//   }

//   return transaction.Commit();
// }

// bool SqlDataStoreAdapter::MigrateV5toV6() {
//   sql::Transaction transaction(&GetDB());
//   if (!transaction.Begin()) {
//     return false;
//   }

//   const char* name = "activity_info";
//   if (GetDB().DoesTableExist(name)) {
//     std::string sql = "ALTER TABLE activity_info RENAME TO activity_info_old;";

//     if (!GetDB().Execute(sql.c_str())) {
//       return false;
//     }

//     sql = "CREATE TABLE ";
//     sql.append(name);
//     sql.append(
//         "("
//         "publisher_id LONGVARCHAR NOT NULL,"
//         "duration INTEGER DEFAULT 0 NOT NULL,"
//         "visits INTEGER DEFAULT 0 NOT NULL,"
//         "score DOUBLE DEFAULT 0 NOT NULL,"
//         "percent INTEGER DEFAULT 0 NOT NULL,"
//         "weight DOUBLE DEFAULT 0 NOT NULL,"
//         "reconcile_stamp INTEGER DEFAULT 0 NOT NULL,"
//         "CONSTRAINT activity_unique "
//         "UNIQUE (publisher_id, reconcile_stamp) "
//         "CONSTRAINT fk_activity_info_publisher_id"
//         "    FOREIGN KEY (publisher_id)"
//         "    REFERENCES publisher_info (publisher_id)"
//         "    ON DELETE CASCADE)");

//     if (!GetDB().Execute(sql.c_str())) {
//       return false;
//     }

//     if (!GetDB().Execute(
//       "CREATE INDEX IF NOT EXISTS activity_info_publisher_id_index "
//       "ON activity_info (publisher_id)")) {
//       return false;
//     }

//     const std::string columns_insert = "publisher_id, "
//                                        "duration, "
//                                        "visits, "
//                                        "score, "
//                                        "percent, "
//                                        "weight, "
//                                        "reconcile_stamp";

//     const std::string columns_select = "publisher_id, "
//                                        "sum(duration) as duration, "
//                                        "sum(visits) as duration, "
//                                        "sum(score) as score, "
//                                        "percent, "
//                                        "weight, "
//                                        "reconcile_stamp";

//     sql = "PRAGMA foreign_keys=off;";
//     sql.append("INSERT INTO activity_info (" + columns_insert + ") "
//                "SELECT " + columns_select + " "
//                "FROM activity_info_old "
//                "GROUP BY publisher_id, reconcile_stamp;");
//     sql.append("DROP TABLE activity_info_old;");
//     sql.append("PRAGMA foreign_keys=on;");

//     bool result = GetDB().Execute(sql.c_str());

//     if (!result) {
//       LOG(ERROR) << "DB: Error with MigrateV5toV6";
//     }
//   }

//   return transaction.Commit();
// }

// bool SqlDataStoreAdapter::Migrate(int version) {
//   switch (version) {
//     case 2: {
//       return MigrateV1toV2();
//     }
//     case 3: {
//       return MigrateV2toV3();
//     }
//     case 4: {
//       return MigrateV3toV4();
//     }
//     case 5: {
//       return MigrateV4toV5();
//     }
//     case 6: {
//       return MigrateV5toV6();
//     }
//     default:
//       return false;
//   }
// }

// sql::InitStatus SqlDataStoreAdapter::EnsureCurrentVersion() {
//   DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

//   // We can't read databases newer than we were designed for.
//   if (meta_table_.GetCompatibleVersionNumber() > GetCurrentVersion()) {
//     LOG(WARNING) << "Publisher info database is too new.";
//     return sql::INIT_TOO_NEW;
//   }

//   const int old_version = GetTableVersionNumber();
//   const int current_version = GetCurrentVersion();
//   const int start_version = old_version + 1;

//   int migrated_version = old_version;
//   for (auto i = start_version; i <= current_version; i++) {
//     if (!Migrate(i)) {
//       LOG(ERROR) << "DB: Error with MigrateV" << (i - 1) << "toV" << i;
//       break;
//     }

//     migrated_version = i;
//   }

//   meta_table_.SetVersionNumber(migrated_version);
//   return sql::INIT_OK;
// }

}  // namespace ledger
