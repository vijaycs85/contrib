<?php
/**
 * @file
 * Defines '' drush command.
 */

/**
 * Implements hook_drush_help().
 */
function geoip_drush_help($section) {
  switch ($section) {
    case 'drush:geoip-update-db':
      return dt('Updates geoip databsse');
  }
}

/**
 * Implements hook_drush_command().
 */
function geoip_drush_command() {
  return array(
    'geoip-update-db' => array(
      'description' => 'Updates the geoip database',
      'aliases' => array('geoip-up'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
      'examples' => array('drush geoip-up' => 'Updates the geoip database'),
      'options' => array(
        'type' => 'Type of geoip database to use ("country" or "city")',
        'example-value' => 'city'),
    ),
  );
}

/**
 * Implements drush_COOMANDFILE_COMMANDNAME().
 */
function drush_geoip_update_db() {

  /* Ensure we have been passed a database type and that it is correct */
  if (!$db_type = drush_get_option('type')) {
    drush_log('You must select either the "city" or "country" database to download', 'error');
    return false;
  }

  switch ($db_type) {
    case 'city':
      $data_filename = 'GeoLiteCity.dat';
      break;

    case 'country':
      $data_filename = 'GeoLiteCountry.dat';
      break;

    default:
      drush_log('You must select either the "city" or "country" database to download', 'error');
      return false;
  }

  /* We need libraries_get_path, else we don't know where to install the database */
  if (!function_exists('libraries_get_path')) {
    drush_log('Function libraries_get_path does not exist', error);
    return false;
  }
  else {
    $destination_path = libraries_get_path('geoip');
    $destination_dir = drush_locate_root() . '/' . $destination_path;
    drush_log(t('destination directory : @destination_dir', array('@destination_dir' => $destination_dir)), 'ok');
  }

  if (!is_file($destination_dir)) {
    mkdir($destination_dir);
  }

  if (!is_writable($destination_dir)) {
    drush_log(t('Destination dir @destination_dir is not writable', array('@destination_dir' => $destination_dir)), 'error');
    return FALSE;
  }

  /* Attempt to download the latest database to a tmp file */
  $db_url = 'http://geolite.maxmind.com/download/geoip/database/' . $data_filename . '.gz';
  $tmp_filename = sys_get_temp_dir() . '/' . $data_filename . '.gz';

  drush_log(t('Downloading @db_url to @tmp_filename', array('@db_url' => $db_url, '@tmp_filename' => $tmp_filename)), 'ok');
  if (file_put_contents($tmp_filename, file_get_contents($db_url)) === FALSE) {
    drush_log('Failed to download file', 'error');
    return FALSE;
  }

  if (!is_readable($tmp_filename)) {
    drush_log('File is not readable', 'error');
    return FALSE;
  }

  /* Attempt to decompress the gzipped database to the destination direction */
  drush_log('Decompressing database', 'ok');
  if (!drush_shell_cd_and_exec($destination_dir, 'gunzip -c ' . $tmp_filename . ' > ' . $destination_dir . '/' . $data_filename)) {
    drush_log(t('Could not gunzip file @tmp_filename', array('@tmp_filename' => $tmp_filename)), 'error');
    return FALSE;
  }

  /* Clean up - delete the temp file, set a drupal variable so that we use the correct database */
  drush_log('Cleaning up', 'ok');
  if (unlink($tmp_filename) === FALSE) {
    drush_log(t('Failed to delete @tmp_filename - you will have to remove this file manually', array('@tmp_filename' => $tmp_filename)), 'notice');
  }

  variable_set('geoip_data_file', $destination_path . '/' . $data_filename );

  return TRUE;

}
