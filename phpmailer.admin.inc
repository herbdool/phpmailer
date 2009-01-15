<?php
// $Id$

/**
 * @file
 * Administrative functions for PHPMailer integration module.
 */

/**
 * Form builder for both the Mime Mail hook and our own menu callback.
 */
function _phpmailer_settings_form() {
  // Override the 'smtp_library' variable.
  if (variable_get('smtp_on', 0)) {
    if (!strpos(variable_get('smtp_library', ''), 'phpmailer')) {
      variable_set('smtp_library', drupal_get_filename('module', 'phpmailer'));
    }
  }
  else {
    if (strpos(variable_get('smtp_library', ''), 'phpmailer')) {
      variable_del('smtp_library');
    }
  }

  $form['enable'] = array(
    '#type' => 'fieldset',
    '#title' => t('Install options'),
  );
  $form['enable']['smtp_on'] = array(
    '#type' => 'radios',
    '#title' => t('Enable global PHPMailer support'),
    '#default_value' => variable_get('smtp_on', 0),
    '#options' => array(1 => 'Yes', 0 => 'No'),
    '#description' => t('To uninstall this module you must turn it off here first.'),
    '#disabled' => (bool)variable_get('mimemail_alter', 0),
  );

  $form['server'] = array(
    '#type' => 'fieldset',
    '#title' => t('SMTP server settings'),
  );
  $form['server']['smtp_host'] = array(
    '#type' => 'textfield',
    '#title' => t('SMTP Server'),
    '#default_value' => variable_get('smtp_host', 'localhost'),
    '#description' => t('The address of the primary SMTP server.'),
    '#required' => TRUE,
  );
  $form['server']['smtp_hostbackup'] = array(
    '#type' => 'textfield',
    '#title' => t('SMTP Backup Server'),
    '#default_value' => variable_get('smtp_hostbackup', ''),
    '#description' => t('An optional address of a SMTP backup server. If the primary server cannot be found this one will be tried.'),
  );
  $form['server']['smtp_port'] = array(
    '#type' => 'textfield',
    '#title' => t('SMTP port'),
    '#size' => 5,
    '#maxlength' => 5,
    '#default_value' => variable_get('smtp_port', '25'),
    '#description' => t('The default SMTP port is 25, GMail uses 465. The default port can be overridden by appending it to the host name: %hostname', array('%hostname' => 'examplehost:465')),
    '#required' => TRUE,
  );
  if (function_exists('openssl_open')) {
    $form['server']['smtp_protocol'] = array(
      '#type' => 'select',
      '#title' => t('Use encrypted protocol'),
      '#default_value' => variable_get('smtp_protocol', 'standard'),
      '#options' => array('' => t('No'), 'ssl://' => t('Use SSL'), 'tls://' => t('Use TLS')),
      '#description' => t('Whether to transmit over a secure connection. GMail uses TLS.'),
    );
  }
  else {
    variable_set('smtp_protocol', '');
  }

  $form['smtp_fromname'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail From name'),
    '#default_value' => variable_get('smtp_fromname', ''),
    '#description' => t('Enter the From name for all messages. If left blank the site name will be used: %sitename', array('%sitename' => variable_get('site_name', 'Drupal'))),
  );

  $form['auth'] = array(
    '#type' => 'fieldset',
    '#title' => t('SMTP Authentication'),
    '#description' => t('Leave blank if your SMTP server does not require authentication.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['auth']['smtp_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => variable_get('smtp_username', ''),
    '#description' => t('SMTP Username.'),
  );
  $form['auth']['smtp_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#default_value' => variable_get('smtp_password', ''),
    '#description' => t('SMTP password.'),
  );

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced SMTP settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['advanced']['smtp_keepalive'] = array(
    '#type' => 'checkbox',
    '#title' => t('SMTP Keep-Alive'),
    '#default_value' => variable_get('smtp_keepalive', 0),
    '#description' => t('Prevents the SMTP connection from being closed after each mail sending.'),
  );
  $form['advanced']['smtp_debug'] = array(
    '#type' => 'select',
    '#title' => t('SMTP Debug level'),
    '#default_value' => variable_get('smtp_debug', 0),
    '#options' => array(0 => t('Disabled')) + drupal_map_assoc(range(1, 4)),
    '#description' => t('Debug the communication with the SMTP server.'),
  );

  // If an address was given, send a test email message.
  $test_address = variable_get('smtp_test_address', '');
  if ($test_address != '') {
    drupal_mail('smtp-test', $test_address, t('Drupal test email'), t('If you receive this message it means your site is capable of sending email.'), variable_get('site_mail', ini_get('sendmail_from')));
    drupal_set_message(t('A test e-mail has been sent to @email. You may want to !check for any error messages.', array('@email' => $test_address, '!check' => l(t('check the logs'), 'admin/logs/watchdog'))));
    variable_del('smtp_test_address');
  }

  $form['test'] = array(
    '#type' => 'fieldset',
    '#title' => t('Send test e-mail'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['test']['smtp_test_address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address to send test email to'),
    '#default_value' => '',
    '#description' => t('Type in an address to have a test email sent there.'),
  );

  return $form;
}

