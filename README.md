# Slack to Drupal

This module integrates with Slack, and imports images posted to a specified Slack channel into Drupal as File entities.

A new file type called ‘Slack Image’  and a new display called ‘Slack Image Display’ will be added. Enable slack image display for the slack image file type at this URL and choose what image style you would like slack images to get rendered with:  
admin/structure/file-types/manage/slack_image/file-display/

Generate a slack token at:
https://api.slack.com/docs/oauth-test-tokens

After you have your token, Upload an image to the channel you would like Drupal to import from. You can view a list of files uploaded to slack (all channels) at:
https://slack.com/api/files.list?token=[your-token]
Your last uploaded file will be the top one. Grab the channel id from the file properties. You will need this to configure slack to drupal module.

Configure slack information at:
admin/config/services/slack_to_drupal/

Once all configuration is completed, run cron to import images from the specified slack channel. You can approve or delete images at:
admin/config/services/slack_to_drupal/approve/

Sponsored by [Zoomdata](http://www.zoomdata.com/) and [Xeno Media, Inc.](http://www.xenomedia.com/)
