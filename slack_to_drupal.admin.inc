<?php
/**
 * @file
 * Admin function for the slack_to_drupal module.
 */

/**
 * Slack to Drupal configruation form.
 */
function slack_to_drupal_admin_config($form, &$form_state) {
  $form['slack_to_drupal_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Slack Client ID'),
    '#description' => t('The client id can be found on your app\'s credentials page'),
    '#default_value' => variable_get('slack_to_drupal_client_id'),
    '#required' => TRUE,
  );

  $form['slack_to_drupal_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Slack Client Secret'),
    '#description' => t('The client secret can be found on your app\'s credentials page'),
    '#default_value' => variable_get('slack_to_drupal_client_secret'),
    '#required' => TRUE,
  );

  $form['slack_to_drupal_folder_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Local path to files'),
    '#description' => t('Path where you would like to save your slack images. Example: slack (will be saved at /sites/default/files/slack)'),
    '#default_value' => variable_get('slack_to_drupal_folder_name'),
    '#required' => TRUE,
  );

  $form['slack_to_drupal_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Slack Client Token'),
    '#description' => t('The token that Slack Provides.'),
    '#default_value' => variable_get('slack_to_drupal_token', ''),
    '#disabled' => TRUE,
  );

  $form['slack_to_drupal_channel_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Slack Channel ID.'),
    '#description' => t('The channel ID or if blank all channels.'),
    '#default_value' => variable_get('slack_to_drupal_channel_id', ''),
    '#disabled' => TRUE,
  );

  $form = system_settings_form($form);

  $form['#submit'][] = 'slack_to_drupal_admin_config_submit';

  return $form;
}

/**
 * Submit handler for slack_to_drupal_admin_config().
 */
function slack_to_drupal_admin_config_submit($form, &$form_state) {
  global $base_url;

  // If file path doesn't exist, create it.
  $file_path = 'public://' . variable_get('slack_to_drupal_folder_name');
  file_prepare_directory($file_path, FILE_CREATE_DIRECTORY);

  $query = array(
    'client_id' => variable_get('slack_to_drupal_client_id'),
    'scope' => 'read',
    'redirect_uri' => $base_url . '/admin/config/services/slack_to_drupal/token',
  );

  $path = 'https://slack.com/oauth/authorize?' . drupal_http_build_query($query);
  drupal_goto($path);
}

/**
 * Sets the Slack token.
 */
function slack_to_drupal_admin_token_config() {
  global $base_url;

  // Get the token from the temporary code.
  if (isset($_GET['code'])) {
    $query = array(
      'client_id' => variable_get('slack_to_drupal_client_id'),
      'client_secret' => variable_get('slack_to_drupal_client_secret'),
      'code' => $_GET['code'],
      'redirect_uri' => $base_url . '/admin/config/services/slack_to_drupal/token',
    );

    $result = drupal_http_request('https://slack.com/api/oauth.access?' . drupal_http_build_query($query));
    if ($result->code == 200) {
      $data = json_decode($result->data);
      if ($data->ok == TRUE) {
        variable_set('slack_to_drupal_token', $data->access_token);
      }
      else {
        drupal_set_message(t('%error', array('%error' => $data->error)), 'error');
      }
    }
  }

  drupal_goto($base_url . '/admin/config/services/slack_to_drupal/channel');

  return '';
}

/**
 * Sets channel.
 */
function slack_to_drupal_admin_channel_config($form, &$form_state) {
  // Get Slack channels.
  $default = array(
    '' => 'All Channels',
  );
  $channels = $default + slack_to_drupal_get_channels();

  $form['slack_to_drupal_channel_id'] = array(
    '#options' => $channels,
    '#type' => 'select',
    '#title' => t('Slack Channel'),
    '#description' => t('Select the Channel you would like to integrate to.'),
    '#default_value' => variable_get('slack_to_drupal_channel_id'),
  );

  $form = system_settings_form($form);

  $form['#submit'][] = 'slack_to_drupal_admin_config_channel_submit';

  return $form;
}

/**
 * Submit handler for slack_to_drupal_admin_channel_config().
 */
function slack_to_drupal_admin_config_channel_submit() {
  global $base_url;
  drupal_goto($base_url . '/admin/config/services/slack_to_drupal/configuration');
}

/**
 * Approve images form.
 */
function slack_to_drupal_approve() {
  $query = new EntityFieldQuery();
  $ids = array();
  $results = array();

  $query->entityCondition('entity_type', 'file')
    ->propertyCondition('type', 'slack_image')
    ->fieldCondition('slack_image_approved', 'value', '0', '=');
  $result = $query->execute();

  if (isset($result['file'])) {
    $ids = array_keys($result['file']);
    // Adds CSS.
    drupal_add_css($data['css'], array('type' => 'inline'));
    drupal_add_css(drupal_get_path('module', 'slack_to_drupal') . '/css/slack_to_drupal.css');
  }

  $fids = array();
  $count = 0;
  foreach ($ids as $id) {
    $file = file_load($id);
    $fileview = file_view($file, 'slack_image_display');
    $results[] = drupal_render($fileview);
    $fids[$count] = $id;
    $count++;
  }

  $form['fids'] = array(
    '#title' =>  'fids',
    '#value' => $fids,
    '#type' => 'value',
  );

  $form['files'] = array(
    '#title' => t('Unapproved Images'),
    '#type' => 'checkboxes',
    '#options' => $results,
  );

  $form['approve'] = array(
    '#type' => 'submit',
    '#value' => t('Approve'),
  );

  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );

  return $form;
}

/**
 * Submit handler for slack_to_drupal_approve().
 */
function slack_to_drupal_approve_submit($form, &$form_state) {
  if (isset($form_state['input']['files'])) {
    foreach ($form_state['input']['files'] as $fid) {
      $image = file_load($form_state['values']['fids'][$fid]);
      if ($form_state['values']['op'] == 'Approve') {
        $image->slack_image_approved['und']['0']['value'] = 1;
        file_save($image);
        drupal_set_message(t('Selected files are approved'));
      }
      else {
        file_delete($image);
        drupal_set_message(t('Selected files are removed'));
      }
    }
  }
}
