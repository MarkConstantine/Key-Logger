using System;
using System.ComponentModel.DataAnnotations;

namespace klog_listener.Entities
{
    public class KeyLog
    {
        [Key]
        public int Id { get; set; }

        public DateTimeOffset InsertedTime { get; set; }

        public string Ip { get; set; }

        public string Text { get; set; }
    }
}
