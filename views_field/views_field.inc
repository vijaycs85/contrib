<?php
/**
 * @file
 * Modifies definitions of base tables, fields, and joins for views.
 *
 * @author Jim Berry ("solotandem", http://drupal.org/user/240748)
 */

/**
 * Adds field table as base table to views.
 */
function views_field_add_base_table(&$data, $field, $module, $base_table) {
  $is_revision = substr($base_table, 0, 15) == 'field_revision_';
  $group = $is_revision ? t('Revision tables') : t('Field tables');

  foreach ($data[$base_table] as $index => $old_item) {
    if (in_array($index, array('table', 'entity_id', 'revision_id', $field['field_name'], $field['field_name'] . '-revision_id'))) {
      // We only want to modify the field "columns" like "value" and "format."
      // Views "moved" the entity_id and revision_id column definitions to:
      // $data[$base_table][$field['field_name']]
      // $data[$base_table][$field['field_name'] . '-revision_id']
      // This seems a bit inconsistent as a naming convention.
      // The entity_id and revision_id array elements also exist.
      continue;
    }
    $column = substr($index, strlen($field['field_name']) + 1);
    if (!in_array($column, $field['settings']['views_base_columns'])) {
      // Ignore columns not selected to be exposed.
      continue;
    }

    // Views now includes a "field" item on the entity_id and revision_id column
    // definitions, eliminating the need for the next line.
    // Add a field handler to views field since the aliased field is not passed
    // as $field to views_get_handler($table, $field, $key, $override = NULL).
    // This has an unintended side effect of exposing the columns in the Views
    // "Add fields" form.
//     $data[$base_table][$index]['field'] = array(
//       'handler' => 'views_handler_field', // @todo Now called views_handler_field_field?
//       'click sortable' => TRUE,
//     );

    // Copy the views generated item.
    $item = $old_item;
    // Use "real field" with field columns so the base table can coexist with
    // standard views "field API" field functionality.
    $item['real field'] = $index;
    $item['group'] = $group;
    foreach (array('argument', 'filter', 'sort') as $type) {
      if (isset($item[$type])) {
        // Remove the field API properties.
        $handler = $item[$type]['handler'];
        $item[$type] = array('handler' => $handler);
        // @todo Maintain ['sort']['allow empty'] = 1?
      }
    }
    // Make the column an official views field that may be selected in UI.
    // @todo The need to set the views field above makes this unnecessary.
    $item['field'] = array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    );
    // Format the title for consistency with other columns.
    // Include the column name for clarity.
    $item['title'] = $field['field_name'] . ' => ' . ucfirst($column);
    $item['title'] = t($item['title']);
    $item['title short'] = $item['title'];
    // Add the new data item.
    $data[$base_table][$index . '_raw'] = $item;
  }

  // Declare a base table.
  $table_type = $is_revision ? ' (revision)' : '';
  // We don't want to set group at the table level, like this:
  // $data[$base_table]['table']['group'] = t('Field tables');
  // @todo With revision table, views does not join on entity_id, revision_id, and entity_type
  $data[$base_table]['table']['base'] = array(
    'field' => $is_revision ? 'revision_id' : 'entity_id',
    'title' => t(str_replace('_', ' ', ucfirst($field['field_name'])) . $table_type),
    'help' => t('Columns from the @field table.', array('@field' => $field['field_name'] . $table_type)),
    'access query tag' => 'user_access', // @todo ???
  );

  // entity_type
  if (in_array($column = 'entity_type', $field['settings']['views_base_columns'])) {
    $title = $field['field_name'] . ' => ' . t('Entity type');
    $data[$base_table][$column] = array(
      'group' => $group,
      'title' => $title,
      'title short' => $title,
      'help' => t('The entity type this data is attached to.'),
      'field' => array(
        'handler' => 'views_handler_field',
        'click sortable' => TRUE,
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_string',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_string',
        'title' => $title, // @todo Why only set title on this item?
        'help' => t('The entity type. This filter does not utilize autocomplete.')
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
    );
  }

  // bundle
  if (in_array($column = 'bundle', $field['settings']['views_base_columns'])) {
    $title = $field['field_name'] . ' => ' . t('Bundle');
    $data[$base_table][$column] = array(
      'group' => $group,
      'title' => $title,
      'title short' => $title,
      'help' => t('The field instance bundle the data is associated with.'),
      'field' => array(
        'handler' => 'views_handler_field',
        'click sortable' => TRUE,
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_string',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_string',
        'help' => t('The field instance bundle the data is associated with. This filter does not utilize autocomplete.')
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
    );
  }

  // deleted
  if (in_array($column = 'deleted', $field['settings']['views_base_columns'])) {
    $title = $field['field_name'] . ' => ' . t('Deleted');
    $data[$base_table][$column] = array(
      'group' => $group,
      'title' => $title,
      'title short' => $title,
      'help' => t('A boolean indicating whether the data item has been deleted.'),
      'field' => array(
        'handler' => 'views_handler_field_numeric',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_numeric',
        'numeric' => TRUE,
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_numeric',
        'numeric' => TRUE,
      ),
    );
  }

  // Handle entity_id and revision_id.
  $is_revision = substr($base_table, 0, 15) == 'field_revision_';
  $id_fields = array('entity', 'revision');
  foreach ($id_fields as $id_field) {
    if (!in_array($id_field . '_id', $field['settings']['views_base_columns'])) {
      continue;
    }
    $title = $field['field_name'] . ' => ' . t(ucfirst($id_field . ' id'));
    // @todo Is this as simple as checking isset($data[$base_table][$field_name])?
    $is_raw = (!$is_revision && $id_field == 'entity') || ($is_revision && $id_field == 'revision');
    $field_name_raw = $field_name = $id_field . '_id';
    $field_name_raw .= $is_raw ? '_raw' : '';
    $data[$base_table][$field_name_raw] = array(
      'real field' => $field_name, // @todo Is it harmful to declare this key when $field_name == $field_name_raw?
      'group' => $group,
      'title' => $title,
      'title short' => $title,
      'title' => $title,
      'title short' => $title,
      'help' => t("The $id_field id."),
      'field' => array(
        'handler' => 'views_handler_field_numeric',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_numeric',
        'numeric' => TRUE,
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_numeric',
        'numeric' => TRUE,
      ),
    );
  }

  // language
  if (in_array($column = 'language', $field['settings']['views_base_columns'])) {
    $title = $field['field_name'] . ' => ' . t('Language');
    $data[$base_table][$column] = array(
      'group' => $group,
      'title' => $title,
      'title short' => $title,
      'help' => t('The language of the data item'),
      'field' => array(
        'handler' => 'views_handler_field_user_language', // @todo ???
        'click sortable' => TRUE,
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_node_language', // @todo ???
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_node_language', // @todo ???
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
    );
  }

  // delta
  if (in_array($column = 'delta', $field['settings']['views_base_columns'])) {
    $title = $field['field_name'] . ' => ' . t('Delta');
    // Views defines this field if multiplicity != 1.
    $field_name_raw = isset($data[$base_table]['delta']) ? 'delta_raw' : 'delta';
    $data[$base_table][$field_name_raw] = array(
      'real field' => 'delta',
      'group' => $group,
      'title' => $title,
      'title short' => $title,
      'help' => t('The sequence number of the data item (multi-value fields).'),
      'field' => array(
        'handler' => 'views_handler_field_numeric',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_numeric',
        'numeric' => TRUE,
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_numeric',
        'numeric' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
    );
  }
}

/**
 * Adds a multi-column join definition between two field tables using the primary key columns.
 *
 * @param array $data
 *   The views data definition.
 * @param string $base_field
 *   Together with $base_type, this defines the base table whose data definition
 *   is to be modified.
 * @param string $join_field
 *   Together with $base_type, this defines the table to join the base table to.
 *   Note: when called by field_group_views, this should be the primary field.
 * @param string $base_type
 *   The prefix applied to $base_field and $join_field to determine the base and
 *   join tables. Allowed values are 'field_data' and 'field_revision.'
 */
function views_field_add_multi_join(&$data, $base_field, $join_field, $base_type = 'field_data') {
  if ($base_type != 'field_data' && $base_type != 'field_revision') {
    return;
  }

  $base_table = $base_type . '_' . $base_field;
  $join_table = $base_type . '_' . $join_field;
  if (!isset($data[$base_table])) {
    return;
  }

  // The primary key columns of a field table.
  $fields = drupal_map_assoc(array('entity_type', 'entity_id', 'revision_id', 'deleted', 'delta', 'language'));
  if ($base_type == 'field_data' || !isset($data[$base_table]['revision_id'])) {
    unset($fields['revision_id']);
  }
  // Define the join.
  $data[$base_table]['table']['join'][$join_table] = array(
    'handler' => 'views_field_join',
    'left_field' => $fields,
    'field' => $fields,
    'type' => 'INNER',
    'extra' => array(
      array('field' => 'deleted', 'value' => 0, 'numeric' => TRUE),
    ),
  );
  // Only expose the join definition to alteration.
  $context = array(
    'base_field' => $base_field,
    'join_field' => $join_field,
    'base_type' => $base_type,
  );
  drupal_alter('views_field_add_multi_join', $data[$base_table]['table']['join'][$join_table], $context);
}
