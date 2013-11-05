<?php

/**
 * @file
 * Admin page callbacks for the flood_control module.
 */

/**
 * Administration settings form.
 *
 * @see system_settings_form()
 */
function flood_control_settings_form() {
  // User module flood events.
  $form['login'] = array(
    '#type' => 'fieldset',
    '#title' => t('Login'),
    '#access' => user_access('administer users'),
  );
  $form['login']['user_failed_login_ip_limit'] = array(
    '#type' => 'select',
    '#title' => t('Failed login (IP) limit'),
    '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 40, 50, 75, 100, 125, 150, 200, 250, 500)),
    '#default_value' => variable_get('user_failed_login_ip_limit', 50),
  );
  $form['login']['user_failed_login_ip_window'] = array(
    '#type' => 'select',
    '#title' => t('Failed login (IP) window'),
    '#options' => array(0 => t('None (disabled)')) + drupal_map_assoc(array(60, 180, 300, 600, 900, 1800, 2700, 3600, 10800, 21600, 32400, 43200, 86400), 'format_interval'),
    '#default_value' => variable_get('user_failed_login_ip_window', 3600),
  );
  $form['login']['user_failed_login_user_limit'] = array(
    '#type' => 'select',
    '#title' => t('Failed login (username) limit'),
    '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 40, 50, 75, 100, 125, 150, 200, 250, 500)),
    '#default_value' => variable_get('user_failed_login_user_limit', 5),
  );
  $form['login']['user_failed_login_user_window'] = array(
    '#type' => 'select',
    '#title' => t('Failed login (username) window'),
    '#options' => array(0 => t('None (disabled)')) + drupal_map_assoc(array(60, 180, 300, 600, 900, 1800, 2700, 3600, 10800, 21600, 32400, 43200, 86400), 'format_interval'),
    '#default_value' => variable_get('user_failed_login_user_window', 21600),
  );

  // Contact module flood events.
  $form['contact'] = array(
    '#type' => 'fieldset',
    '#title' => t('Contact forms'),
    '#access' => user_access('administer contact forms'),
  );
  $form['contact']['contact_threshold_limit'] = array(
    '#type' => 'select',
    '#title' => t('Sending e-mails limit'),
    '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 40, 50, 75, 100, 125, 150, 200, 250, 500)),
    '#default_value' => variable_get('contact_threshold_limit', 5),
  );
  $form['contact']['contact_threshold_window'] = array(
    '#type' => 'select',
    '#title' => t('Sending e-mails window'),
    '#options' => array(0 => t('None (disabled)')) + drupal_map_assoc(array(60, 180, 300, 600, 900, 1800, 2700, 3600, 10800, 21600, 32400, 43200, 86400), 'format_interval'),
    '#default_value' => variable_get('contact_threshold_window', 3600),
  );

  // Show a message if the user does not have any access to any options.
  if (!element_get_visible_children($form)) {
    $form['nothing'] = array(
      '#markup' => '<p>' . t('Sorry, there are no flood control options for you to configure.') . '</p>',
    );
  }
  else {
    $form = system_settings_form($form);
  }

  return $form;
}

/**
 * Admin failed logins management form
 *
 * @see flood_control_blocked_form_submit()
 */
function flood_control_blocked_form() {
  $form['blocked_users'] = array(
    '#type' => 'fieldset',
    '#title' => t('Blocked Users'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('Check the box next to a user and submit the form to allow that user to login.'),
  );
  $form['blocked_users']['users'] = array(
    '#type' => 'tableselect',
    '#header' => flood_control_blocked_users_header(),
    '#options' => flood_control_blocked_users(),
  );

  $form['blocked_ips'] = array(
    '#type' => 'fieldset',
    '#title' => t('Blocked IP Addresses'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('Check the box next to an IP Address and submit the form to allow logins from that IP address.'),
  );
  $form['blocked_ips']['ips'] = array(
    '#type' => 'tableselect',
    '#header' => flood_control_blocked_ips_header(),
    '#options' => flood_control_blocked_ips(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Submit handler for the flood control blocked form.
 *   Delete any checked users/IPs from the flood table.
 *
 * @see flood_control_blocked_form()
 *
 * @param $form
 * @param $form_state
 */
function flood_control_blocked_form_submit($form, &$form_state) {
  // Clear any checked users.
  foreach ($form_state['values']['users'] as $flood_uid => $checked) {
    if ($checked != 0) {
      try {
        // This user has been selected to be cleared.
        db_delete('flood')
          ->condition('event', 'failed_login_attempt_user')
          ->condition('identifier', $flood_uid . '-%', 'LIKE')
          ->execute();
        $account = user_load($flood_uid);
        drupal_set_message(t('Flood control entries have been cleared for the user %name, and they may now log in.', array('%name' => $account->name)));
      }
      catch (Exception $e) {
        drupal_set_message(t('I was unable to clear the flood control entries for the user %name, please try again.', array('%name' => $account->name)));
        // Log the exception to watchdog.
        watchdog_exception('type', $e);
      }

    }
  }

  // Clear any checked IPs.
  foreach ($form_state['values']['ips'] as $flood_ip => $checked) {
    if ($checked != 0) {
      try {
        // This IP has been selected to be cleared.
        db_delete('flood')
          ->condition('event', 'failed_login_attempt_ip')
          ->condition('identifier', $flood_ip, '=')
          ->execute();
        drupal_set_message(t('Flood control entries have been cleared for the IP Address %ip, and users may now log in from it.', array('%ip' => $flood_ip)));
      }
      catch (Exception $e) {
        drupal_set_message(t('I was unable to clear the flood control entries for the user %name, please try again.', array('%name' => $account->name)));
        // Log the exception to watchdog.
        watchdog_exception('type', $e);
      }

    }
  }
}
