#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"

void prepareViews(THD* thd, string oldSchema, string newSchema, vector<column> matches) {
    Ed_connection c(thd);
    executeQuery(c, "CREATE TABLE " + newSchema);
}
